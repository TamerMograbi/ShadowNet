#include <nori/integrator.h>
#include <nori/scene.h>
#include <nori/sampler.h>
#include <nori/bsdf.h>
#include <nori/emitter.h>
#include <math.h>

NORI_NAMESPACE_BEGIN

class PathIntegrator : public Integrator {
public:
	PathIntegrator(const PropertyList &props) {
	}

	void preprocess(const Scene * scene) {
		emitterMeshes = scene->getEmitterMeshes();
	}

	Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const {
		return pathTracer(scene, sampler, ray, 0);
	}

	//returns true if the x and y are mutually visible
	//if ray from x to y hits anything other than the y then it's invisible
	//input : scene, point x on mesh, point y on emitter, pointer to mesh that y is on.
	bool isVisible(const Scene *scene, const Point3f& x, const Point3f& y, const Mesh* yMesh) const
	{
		Ray3f ray(x, y - x);
		Intersection its;
		//not using shadowRay here as i need to know what mesh it hit
		scene->rayIntersect(ray, its);//it will definitely at least hit the mesh that y is on so no need to check return value
		return yMesh == its.mesh;//only returns true if ray intersected with y (and so missed all other meshes)

	}

	Color3f pathTracer(const Scene *scene, Sampler *sampler, const Ray3f &ray, int k) const {

		Intersection its;
		if (!scene->rayIntersect(ray, its))
			return Color3f(0.0f);

		Point3f x = its.p;

		// if we directly hit a light source then return emission
		if (k == 0 && its.mesh->isEmitter())
		{
			return its.mesh->getEmitter()->getRadiance();
		}
		const BSDF * bsdf = its.mesh->getBSDF();
		if (!bsdf->isDiffuse())
		{
			BSDFQueryRecord record(its.shFrame.toLocal((-ray.d).normalized()));
			record.measure = EMeasure::ESolidAngle;
			Color3f bsdfSample = bsdf->sample(record, sampler->next2D());
			//now record would hold the refracted/reflected dir in wo
			float sampleStuck = sampler->next1D();
			if (sampleStuck >= 0.95f)
			{
				//in case algorithm gets stuck in reflection/refraction events
				return Color3f(0.0f);
			}
			Intersection its2;
			Color3f lightEmission = 0.f;
			if (scene->rayIntersect(Ray3f(x, its.shFrame.toWorld(record.wo)), its2))
			{
				if (its2.mesh->isEmitter())
				{
					lightEmission = its2.mesh->getEmitter()->getRadiance();
				}
			}
			return lightEmission* bsdfSample + bsdfSample * pathTracer(scene, sampler, Ray3f(x, its.shFrame.toWorld(record.wo)), k + 1) / 0.95f;
		}

		//now we sample a point on one of the emitters
		float sample = sampler->next1D();

		//we choose point on emitter
		int emitterIdx = sample * emitterMeshes.size();

		//surfaceSample includes sampled position on mesh, normal at position, and 1/(mesh surface area)
		SurfaceSample surfSample = emitterMeshes[emitterIdx]->getSurfaceSample(sampler->next2D(), sampler->next1D());

		//the record holds ray from emitter to x on mesh and ray from camera.
		BSDFQueryRecord record(its.shFrame.toLocal((surfSample.p - x).normalized()), its.shFrame.toLocal((-ray.d).normalized()), EMeasure::ESolidAngle);
		Color3f fr = bsdf->eval(record);
		float lightPdfArea = surfSample.pdf / (float) emitterMeshes.size();

		Color3f Le = emitterMeshes[emitterIdx]->getEmitter()->getRadiance();
		bool visible = isVisible(scene, x, surfSample.p, emitterMeshes[emitterIdx]);

		float CosWithLight = surfSample.n.dot((x - surfSample.p).normalized());

		//instead of calculating geometric term, it will be easier and more efficient to calculate cosTheta and lightPdfAngle seperately
		float cosTheta = its.shFrame.cosTheta(its.shFrame.toLocal((surfSample.p - x).normalized()));
		float lightPdfAngle = lightPdfArea * (x-surfSample.p).squaredNorm() / CosWithLight;
		Color3f lightSample = (cosTheta >= 0 && CosWithLight > 0 && visible) ? Le * fr * cosTheta / lightPdfAngle : Color3f(0.f);

		BSDFQueryRecord hypotheticalRec(its.shFrame.toLocal(-ray.d.normalized()), its.shFrame.toLocal((surfSample.p-x).normalized()), ESolidAngle);
		//bsdf->pdf(hypotheticalRec) = hypothetically the density with which the bsdf will choose same random point on light
		float wLight = lightPdfAngle / (lightPdfAngle + bsdf->pdf(hypotheticalRec));
		wLight = std::isfinite(wLight) ? wLight : 0.f;

		//now that we have both wLight and lightSample value, we will calculate wBRDF and bsdfSample

		BSDFQueryRecord bsdfRec(its.shFrame.toLocal(-ray.d.normalized()));
		Color3f bsdfSample = bsdf->sample(bsdfRec, sampler->next2D());
		Color3f Le2 = 0.f;
		lightPdfArea = 0.f; // we want to reuse this variable
		float lightPdfAngleHypothetical = 0.f;
		// check if this new sampled direction hits an emitter
		Intersection its2;
		bool intersectionFound = scene->rayIntersect(Ray3f(x, its.shFrame.toWorld(bsdfRec.wo)), its2);
		if (intersectionFound && its2.mesh->isEmitter()) {
			CosWithLight = its2.shFrame.cosTheta(its2.shFrame.toLocal((x - its2.p).normalized()));
			//if cosWithLight is less than 0 then we are on the side of the object that doesn't emit light and so we leave everything to be 0
			if (CosWithLight > 0) {
				Le2 = its2.mesh->getEmitter()->getRadiance();
				lightPdfArea = its2.mesh->getMeshSurfaceArea() / (float)emitterMeshes.size();
				//again we convert light pdf to solid angles
				//lightPdfAngleHypothetical is the density with which we would have randomly chosen the same point on the same light in solid angles
				lightPdfAngleHypothetical = lightPdfArea * (x - its2.p).squaredNorm() / CosWithLight;
			}
		}

		float wBRDF = bsdf->pdf(bsdfRec) / (bsdf->pdf(bsdfRec) + lightPdfAngleHypothetical);
		wBRDF = std::isfinite(wBRDF) ? wBRDF : 0.f;

		//now we also have wBRDF and bsdfSample so we have everything we need

		//terminate by probablity q which is 0 for first two vertices
		float q = (k <= 1) ? 0.f : 0.5f;
		if (sampler->next1D() < q) { return Color3f(0.f); }

		return lightSample * wLight + Le2 * bsdfSample * wBRDF + pathTracer(scene, sampler, Ray3f(x, its.shFrame.toWorld(bsdfRec.wo)), k + 1) * bsdfSample / ((1.f - q));
	}

	std::string toString() const {
		return "PathIntegrator[]";
	}
private:
	std::vector<Mesh *> emitterMeshes;
};

NORI_REGISTER_CLASS(PathIntegrator, "path");
NORI_NAMESPACE_END