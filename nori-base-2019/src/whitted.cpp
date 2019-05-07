#include <nori/integrator.h>
#include <nori/scene.h>
#include <nori/sampler.h>
#include <nori/bsdf.h>
#include <nori/emitter.h>

NORI_NAMESPACE_BEGIN

class WhittedIntegrator : public Integrator {
public:
	WhittedIntegrator(const PropertyList &props)
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
		Intersection its;
		if (!scene->rayIntersect(ray, its))
		{
			return Color3f(0.f);
		}
		Point3f x = its.p; //where the ray hits the mesh

		const BSDF* bsdf = its.mesh->getBSDF();
		if (its.mesh->isEmitter())
		{
			return its.mesh->getEmitter()->getRadiance();
		}
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
			return (1.f / 0.95f) * bsdfSample * Li(scene, sampler, Ray3f(x, its.shFrame.toWorld(record.wo)));
		}

		//now we sample a point on one of the emitters
		float sample = sampler->next1D();

		//we choose point on emitter
		int emitterIdx = sample * emitterMeshes.size();

		//surfaceSample includes sampled position on mesh, normal at position, and 1/(mesh surface area)
		SurfaceSample surfSample = emitterMeshes[emitterIdx]->getSurfaceSample(sampler->next2D(), sampler->next1D());

		//make sure that light is only emitted from the positive direction of the n on the emitter mesh
		if ((surfSample.n.dot(x - surfSample.p)) <= 0.0f) { return Color3f(0.0f); }

		//the record holds ray from emitter to x on mesh and ray from camera.
		BSDFQueryRecord record(its.shFrame.toLocal((surfSample.p - x).normalized()), its.shFrame.toLocal((-ray.d).normalized()), EMeasure::ESolidAngle);
		Color3f fr = bsdf->eval(record);

		//now to calculate Le(y,y->x)
		Color3f Le = emitterMeshes[emitterIdx]->getEmitter()->getRadiance();//radiance is uniform on the entire area
		if (!isVisible(scene, x, surfSample.p, emitterMeshes[emitterIdx]))
		{
			return Color3f(0.0f);
		}
		//now we calculate G(x<->y)
		Vector3f nx = its.shFrame.n;
		Vector3f ny = surfSample.n;
		nx.normalize();
		ny.normalize();
		Point3f y = surfSample.p;
		float g = geometricTerm(nx, ny, x, y);

		return (g * fr * Le) / surfSample.pdf;
	}

	std::string toString() const {
		return "WhittedIntegrator[]";
	}

	EIntegratorType getIntegratorType() const
	{
		return EIntegratorType::EWhitted;
	}
private:
	std::vector<Mesh*> emitterMeshes;

};

NORI_REGISTER_CLASS(WhittedIntegrator, "whitted");
NORI_NAMESPACE_END