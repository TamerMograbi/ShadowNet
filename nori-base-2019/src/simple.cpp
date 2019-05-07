#include <nori/integrator.h>
#include <nori/scene.h>
#include <nori/bsdf.h>

NORI_NAMESPACE_BEGIN

class SimpleIntegrator : public Integrator {
public:
	SimpleIntegrator(const PropertyList &props)
	{
		position = props.getPoint("position");
		energy = props.getColor("energy");
		cout << "energy = " << energy.toString() << endl;
		first = true;
		//min x max x, min y max y, min z max z
		minMaxVector = { 100,-100,100,-100,100,-100 };
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
		if (first)
		{
			first = false;
			//cout << " pos = " << x << endl;
		}
		//index 0 holds min x of mesh
		minMaxVector[0] = (x[0] < minMaxVector[0]) ? x[0] : minMaxVector[0];
		//index 1 holds max x of mesh
		minMaxVector[1] = (x[0] > minMaxVector[1]) ? x[0] : minMaxVector[1];
		//index 2 holds min y
		minMaxVector[2] = (x[1] < minMaxVector[2]) ? x[1] : minMaxVector[2];
		//index 3 holds max y
		minMaxVector[3] = (x[1] > minMaxVector[3]) ? x[1] : minMaxVector[3];
		//index 4 holds min z
		minMaxVector[4] = (x[2] < minMaxVector[4]) ? x[2] : minMaxVector[4];
		//index 5 holds max z
		minMaxVector[5] = (x[2] > minMaxVector[5]) ? x[2] : minMaxVector[5];
		//cout << " pos = " << x << endl;
		const BSDF* bsdf = its.mesh->getBSDF();
		Vector3f v = position - x;
		Normal3f n = its.shFrame.n;
		n.normalize();
		v.normalize();
		Vector3f xMinusP = x - position;
		float cosRes = v.dot(n);

		BSDFQueryRecord record(its.shFrame.toLocal((position - x).normalized()), its.shFrame.toLocal((-ray.d).normalized()), EMeasure::ESolidAngle);
		if (!visiblity(scene, x))
		{
			return Color3f(0.0f);
		}
		return (energy / (4 * M_PI*M_PI)) * bsdf->eval(record) * ((cosRes > 0 ? cosRes : 0) / xMinusP.squaredNorm());
	}

	std::string toString() const {
		return "SimpleIntegrator[]";
	}

	EIntegratorType getIntegratorType() const
	{
		return EIntegratorType::ESimple;
	}

	std::vector<float> getMinMaxVector() const
	{
		return minMaxVector;
	}

	Point3f position; // postion of point light source
	Color3f energy; // energy of point light source
	mutable bool first;
	mutable std::vector<float> minMaxVector;
	


};

NORI_REGISTER_CLASS(SimpleIntegrator, "simple");
NORI_NAMESPACE_END