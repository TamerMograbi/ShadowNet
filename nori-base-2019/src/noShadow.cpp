#include <nori/integrator.h>
#include <nori/scene.h>
#include <nori/bsdf.h>

NORI_NAMESPACE_BEGIN

class NosShadowIntegrator : public Integrator {
public:
	NosShadowIntegrator(const PropertyList &props)
	{
		position = props.getPoint("position");
		energy = props.getColor("energy");
		cout << "energy = " << energy.toString() << endl;
	}

	//point x will be visible only if light reaches it
	bool visiblity(const Scene *scene, Point3f x) const
	{
		Vector3f dir = position - x;//direction from point to light source
		dir.normalize();
		Ray3f ray(x, dir);
		return !scene->rayIntersect(ray);//if ray intersects, then it means that it hit another point in the mesh
		//while on it's way to the light source. so x won't recieve light
	}

	Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const
	{
		Intersection its;
		if (!scene->rayIntersect(ray, its))
		{
			return Color3f(0.0f);
		}
		Point3f x = its.p; //where the ray hits the mesh
		const BSDF* bsdf = its.mesh->getBSDF();
		Vector3f v = position - x;
		Normal3f n = its.shFrame.n;
		n.normalize();
		v.normalize();
		Vector3f xMinusP = x - position;
		float cosRes = v.dot(n);

		BSDFQueryRecord record(its.shFrame.toLocal((position - x).normalized()), its.shFrame.toLocal((-ray.d).normalized()), EMeasure::ESolidAngle);
		return (energy / (4 * M_PI*M_PI)) * bsdf->eval(record) * ((cosRes > 0 ? cosRes : 0) / xMinusP.squaredNorm());
	}

	std::string toString() const {
		return "NosShadowIntegrator[]";
	}

	EIntegratorType getIntegratorType() const
	{
		return EIntegratorType::ENoShadows;
	}

	Point3f position; // postion of point light source
	Color3f energy; // energy of point light source


};

NORI_REGISTER_CLASS(NosShadowIntegrator, "noShadow");
NORI_NAMESPACE_END