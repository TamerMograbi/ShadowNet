#include <nori/integrator.h>
#include <nori/scene.h>
#include <nori/warp.h>
#include <pcg32.h>

NORI_NAMESPACE_BEGIN

class AoIntegrator : public Integrator {
public:
	AoIntegrator(const PropertyList &props)
	{
		//no properties
	}

	bool visiblity(const Scene * scene, const Vector3f& wi, const Point3f& x) const
	{
		Ray3f ray(x, wi);
		return !scene->rayIntersect(ray);//if ray hits another point in mesh then light from this direction doesn't reach it
	}
	Color3f Li(const Scene *scene, Sampler *sampler, const Ray3f &ray) const
	{
		Intersection its;
		if (!scene->rayIntersect(ray, its))
		{
			return Color3f(0.0f);
		}
		Point3f x = its.p; //where the ray hits the mesh
		Color3f res(0.0f);
		{
			Point2f sample(sampler->next2D());
			Vector3f wiSample(Warp::squareToCosineHemisphere(sample));
			//convert from local to world coordinates
			wiSample = its.geoFrame.toWorld(wiSample);

			if (visiblity(scene, wiSample, x)) {
				res = Color3f(1.f);
			}
		}
		return res;
		
	}
	std::string toString() const {
		return "AoIntegrator[]";
	}
};


NORI_REGISTER_CLASS(AoIntegrator, "ao");
NORI_NAMESPACE_END