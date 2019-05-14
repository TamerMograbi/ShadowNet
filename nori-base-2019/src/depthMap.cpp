#include <nori/integrator.h>
#include <nori/scene.h>
#include <nori/bsdf.h>

NORI_NAMESPACE_BEGIN

class depthMapIntegrator : public Integrator {
public:
	depthMapIntegrator(const PropertyList &props)
	{
		position = props.getPoint("position");
		energy = props.getColor("energy");
		maxDist = 0.f;
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
		
		float distance = (x - ray.o).norm();//distance beween origin of ray to position on mesh

		return Color3f(2.0f/distance);
	}


	std::string toString() const {
		return "depthMapIntegrator[]";
	}

	EIntegratorType getIntegratorType() const
	{
		return EIntegratorType::EDepthMap;
	}

	Point3f position; // postion of point light source
	Color3f energy; // energy of point light source
	float maxDist;


};

NORI_REGISTER_CLASS(depthMapIntegrator, "depthMap");
NORI_NAMESPACE_END