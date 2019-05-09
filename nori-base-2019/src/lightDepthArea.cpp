#include <nori/integrator.h>
#include <nori/scene.h>
#include <nori/bsdf.h>

NORI_NAMESPACE_BEGIN

class lightDepthAreaIntegrator : public Integrator {
public:
	lightDepthAreaIntegrator(const PropertyList &props)
	{
	}
    void preprocess(const Scene *scene)
    {
        emitterMeshes = scene->getEmitterMeshes();
        const MatrixXf vertices = emitterMeshes[0]->getVertexPositions();

        for (int colIdx = 0; colIdx < vertices.cols(); colIdx++)
        {
            meshCenter += vertices.col(colIdx);
        }
        meshCenter /= vertices.cols();
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
        Point3f x = its.p; //where the ray hits the mesh
        Vector3f intersectionToLight = (meshCenter - x).normalized();
        float scaledX = scaleToAB(intersectionToLight[0], 0, 1);
        float scaledY = scaleToAB(intersectionToLight[1], 0, 1);
        float scaledZ = scaleToAB(intersectionToLight[2], 0, 1);
        //we scale from -1 to 1 (which is the range that the coordinates of a normalized direction can be in) to 0 2
        return Color3f(scaledX, scaledY, scaledZ);
	}


	std::string toString() const {
		return "lightDepthAreaIntegrator[]";
	}

	EIntegratorType getIntegratorType() const
	{
		return EIntegratorType::ELightDepthArea;
	}

    std::vector<Mesh *> emitterMeshes;
    Point3f meshCenter;

};

NORI_REGISTER_CLASS(lightDepthAreaIntegrator, "lightDepthArea");
NORI_NAMESPACE_END