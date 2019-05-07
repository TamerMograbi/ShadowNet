#include <nori/integrator.h>
#include <nori/scene.h>
#include <nori/sampler.h>
#include <nori/bsdf.h>
#include <nori/emitter.h>

NORI_NAMESPACE_BEGIN

class PathSimpleIntegrator : public Integrator {
public:
	PathSimpleIntegrator(const PropertyList &props)
	{
	}

	void preprocess(const Scene *scene)
	{
		emitterMeshes = scene->getEmitterMeshes();
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
	//we assune that nx and ny are normalized
	float geometricTerm(const Vector3f& nx, const Vector3f& ny, const Point3f& x, const Point3f& y) const
	{
		Vector3f xToY = (y - x).normalized();
		Vector3f yToX = (x - y).normalized();
		return  (std::abs(nx.dot(xToY))*std::abs(ny.dot(yToX))) / ((x - y).squaredNorm());
	}

	Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const
	{
		return pathTracer(scene, sampler, ray, 0);
	}

	Color3f pathTracer(const Scene *scene, Sampler *sampler, const Ray3f &ray, int k) const
	{
		Intersection its;
		if (!scene->rayIntersect(ray, its))
		{
			return Color3f(0.f);
		}
		Point3f x = its.p; //where the ray hits the mesh

		//if we directly hit a light source then return emission
		if (k == 0 && its.mesh->isEmitter())
		{
			return its.mesh->getEmitter()->getRadiance();
		}

		const BSDF* bsdf = its.mesh->getBSDF();
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
			return lightEmission + bsdfSample * pathTracer(scene, sampler, Ray3f(x, its.shFrame.toWorld(record.wo)), k) / 0.95f;
		}

		//now we sample a point on one of the emitters
		float sample = sampler->next1D();

		//we choose point on emitter
		int emitterIdx = sample * emitterMeshes.size();

		//surfaceSample includes sampled position on mesh, normal at position, and 1/(mesh surface area)
		SurfaceSample surfSample = emitterMeshes[emitterIdx]->getSurfaceSample(sampler->next2D(), sampler->next1D());

		//make sure that light is only emitted from the positive direction of the n on the emitter mesh
		
		//should only return 0 if it's camera ray. so commented out for now

		//the record holds ray from emitter to x on mesh and ray from camera.
		BSDFQueryRecord record(its.shFrame.toLocal((surfSample.p - x).normalized()), its.shFrame.toLocal((-ray.d).normalized()), EMeasure::ESolidAngle);
		Color3f fr = bsdf->eval(record);

		//now to calculate Le(y,y->x)
		Color3f Le = emitterMeshes[emitterIdx]->getEmitter()->getRadiance();//radiance is uniform on the entire area
		bool visible = isVisible(scene, x, surfSample.p, emitterMeshes[emitterIdx]);
		//now we calculate G(x<->y)
		Vector3f nx = its.shFrame.n;
		Vector3f ny = surfSample.n;
		nx.normalize();
		ny.normalize();
		Point3f y = surfSample.p;
		float g = geometricTerm(nx, ny, x, y);

		float q = (k <= 1) ? 0.f : 0.5f;
		if (sampler->next1D() < q) { return 0.f; }

		BSDFQueryRecord rec(its.shFrame.toLocal((-ray.d).normalized()));
		rec.measure = EMeasure::ESolidAngle;
		Color3f bsdfSample = bsdf->sample(rec, sampler->next2D());
		Color3f val = (g * fr * Le) / surfSample.pdf;
		if (!visible || ((k == 0) && ((surfSample.n.dot(x - surfSample.p)) <= 0.0f)))
		{
			val = Color3f(0.f);
		}
		return val + bsdfSample * pathTracer(scene, sampler, Ray3f(x, its.shFrame.toWorld(rec.wo)), k + 1) / (1.f - q);
	}

	std::string toString() const {
		return "WhittedIntegrator[]";
	}
private:
	std::vector<Mesh*> emitterMeshes;

};

NORI_REGISTER_CLASS(PathSimpleIntegrator, "path_simple");
NORI_NAMESPACE_END