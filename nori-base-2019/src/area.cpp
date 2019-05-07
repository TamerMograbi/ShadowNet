#include <nori/emitter.h>

NORI_NAMESPACE_BEGIN

class AreaLight : public Emitter {
public:
	AreaLight(const PropertyList &props)
	{
		radiance = props.getColor("radiance");
	}

	std::string toString() const {
		return "AreaLight[]";
	}

	Color3f getRadiance() const
	{
		return radiance;
	}


private:
	Color3f radiance;
};

NORI_REGISTER_CLASS(AreaLight, "area");
NORI_NAMESPACE_END