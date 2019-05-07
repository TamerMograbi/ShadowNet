#include <nori/integrator.h>
#include <nori/scene.h>
#include <nori/bsdf.h>

NORI_NAMESPACE_BEGIN

class lightDepthAreaIntegrator : public Integrator {
public:
	lightDepthAreaIntegrator(const PropertyList &props)
	{
	}

	//point x will be visible only if light reaches it
	//bool visiblity(const Scene *scene, Point3f x) const
	//{
	//	Vector3f dir = position - x;//direction from point to light source
	//	dir.normalize();
	//	Ray3f ray(x, dir);
	//	return !scene->rayIntersect(ray);//if ray intersects, then it means that it hit another point in the mesh
	//	//while on it's way to the light source. so x won't recieve light
	//}
	//scale value x from range [-1,1] to [a,b]
	float scaleToAB(float x, float a, float b) const
	{
		return (b - a)*(x + 1) / 2 + a;
	}

	Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const
	{
		Intersection its;
		if (!scene->rayIntersect(ray, its))
		{
			return Color3f(0.f);
		}
		if (its.mesh->isEmitter())
		{
			float distance = (its.p - ray.o).norm();//distance beween origin of ray to position on mesh

			return Color3f(1.f / distance);
		}
		return Color3f(0.f);
	}


	std::string toString() const {
		return "lightDepthAreaIntegrator[]";
	}

	EIntegratorType getIntegratorType() const
	{
		return EIntegratorType::ELightDepthArea;
	}



};

NORI_REGISTER_CLASS(lightDepthAreaIntegrator, "lightDepthArea");
NORI_NAMESPACE_END