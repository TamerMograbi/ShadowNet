/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob

    Nori is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Nori is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <nori/bsdf.h>
#include <nori/frame.h>

NORI_NAMESPACE_BEGIN



/// Ideal dielectric BSDF
class Dielectric : public BSDF {
public:
    Dielectric(const PropertyList &propList) {
        /* Interior IOR (default: BK7 borosilicate optical glass) */
        m_intIOR = propList.getFloat("intIOR", 1.5046f);

        /* Exterior IOR (default: air) */
        m_extIOR = propList.getFloat("extIOR", 1.000277f);
    }

    Color3f eval(const BSDFQueryRecord &) const {
        /* Discrete BRDFs always evaluate to zero in Nori */
        return Color3f(0.0f);
    }

    float pdf(const BSDFQueryRecord &) const {
        /* Discrete BRDFs always evaluate to zero in Nori */
        return 0.0f;
    }

	//calculate the refracted direction using Snell's law
	bool getRefractedDir(BSDFQueryRecord &bRec, float n1, float n2) const
	{	
		float cosThetaWi = Frame::cosTheta(bRec.wi);
		Vector3f n = Vector3f(0, 0, 1);
		n = (cosThetaWi < 0.f) ? -n : n;//n needs to be in the same hemisphere as wi for the computation to work

		cosThetaWi = std::abs(cosThetaWi);
		//we need cosThetaT in order to calculate t(wi)
		float sinThetaWi = std::sqrt(1.f - cosThetaWi * cosThetaWi);
		float sinThetaWt = ((n1) / (n2))*sinThetaWi;//using Snell's law

		float cosThetaWt = std::sqrt(1 - sinThetaWt* sinThetaWt);//cos^2x+sin^2x=1
		//now that we have cosThetaWt, we can calculate the refracted dir

		bRec.wo = (n1 / n2)*(-bRec.wi) + ((n1 / n2)*cosThetaWi - cosThetaWt)*n;
		return true;
	}

	//calculate Fr(wi) Reflection coefficient. (1-Fr) = Refraction coeff 
	float getFr(const Vector3f &w) const
	{
		float n1 = m_extIOR;
		float n2 = m_intIOR;
		float cosThetaWi = Frame::cosTheta(w);
		bool enteringObj = cosThetaWi > 0.f; //means that wi is in the same hemisphere as n.
		//if this isn't the case then wi is in the object.

		//if the ray is inside the object and going out then we need to flip the refractive indices
		//as n1 needs to be where the ray is at the begining
		if (!enteringObj)
		{
			std::swap(n1, n2);
			cosThetaWi = std::abs(cosThetaWi);//since we flipped the indices
		}
		//the 2 r equations need cos(thetaT) which we'll calculate using Snell's law
		float sinThetaWi = std::max(0.f, std::sqrt(1 - cosThetaWi * cosThetaWi));//1=sin^2x+cos^2x
		//now according to Snell's law
		float sinThetaWt = (n1 / n2) * sinThetaWi;

		float cosThetaWt = std::max(0.f, std::sqrt(1 - sinThetaWt * sinThetaWt));//1=sin^2x+cos^2x

		float rParallel = (n2*cosThetaWi - n1 * cosThetaWt) / (n2*cosThetaWi + n1 * cosThetaWt);

		float rPerpendicular = (n1*cosThetaWi - n2 * cosThetaWt) / (n1*cosThetaWi + n2 * cosThetaWt);

		return 0.5f*(rParallel * rParallel + rPerpendicular * rPerpendicular);

	}

	float refractiveBRDF(BSDFQueryRecord &bRec) const
	{
		float n1 = m_extIOR;
		float n2 = m_intIOR;
		float cosThetaWi = Frame::cosTheta(bRec.wi);
		bool enteringObj = cosThetaWi > 0.f;
		if (!enteringObj)
		{
			std::swap(n1, n2);
		}
		//now we calculate the refracted dir (wo will hold the refracted dir)
		getRefractedDir(bRec, n1, n2);
		return 1.f;
	}

	float reflectionBRDF(BSDFQueryRecord &bRec) const
	{
		//we calculate the reflected dir
		bRec.wo = Vector3f(-bRec.wi[0], -bRec.wi[1], bRec.wi[2]);//since we are in local coordinates r=-wo+2wo.z*(0,0,1)=(-wi.x,-wo.y,wo.z)

		return 1.f;
	}
	
    Color3f sample(BSDFQueryRecord &bRec, const Point2f &sample) const {
		float Fr = getFr(bRec.wi);
		//choose either specular or refractive according to the value of the sample
		//since Fr is the reflection coeff, then if sample is less than Fr, we use reflectionBRDF
		if (sample[0] < Fr) {
			return Color3f(reflectionBRDF(bRec));
		}
		else {
			return Color3f(refractiveBRDF(bRec));
		}
    }

    std::string toString() const {
        return tfm::format(
            "Dielectric[\n"
            "  intIOR = %f,\n"
            "  extIOR = %f\n"
            "]",
            m_intIOR, m_extIOR);
    }
private:
    float m_intIOR, m_extIOR;
};

NORI_REGISTER_CLASS(Dielectric, "dielectric");
NORI_NAMESPACE_END
