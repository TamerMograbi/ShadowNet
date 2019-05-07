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
#include <nori/warp.h>

NORI_NAMESPACE_BEGIN

class Microfacet : public BSDF {
public:
    Microfacet(const PropertyList &propList) {
        /* RMS surface roughness */
        m_alpha = propList.getFloat("alpha", 0.1f);

        /* Interior IOR (default: BK7 borosilicate optical glass) */
        m_intIOR = propList.getFloat("intIOR", 1.5046f);

        /* Exterior IOR (default: air) */
        m_extIOR = propList.getFloat("extIOR", 1.000277f);

        /* Albedo of the diffuse base material (a.k.a "kd") */
        m_kd = propList.getColor("kd", Color3f(0.5f));

        /* To ensure energy conservation, we must scale the 
           specular component by 1-kd. 

           While that is not a particularly realistic model of what 
           happens in reality, this will greatly simplify the 
           implementation. Please see the course staff if you're 
           interested in implementing a more realistic version 
           of this BRDF. */
        m_ks = 1 - m_kd.maxCoeff();
    }

	float xPlus(float c) const
	{
		return (c > 0.f) ? 1.f : 0.f;
	}

	float G1(Vector3f wv, Vector3f wh) const
	{
		Vector3f n = Vector3f(0.f, 0.f, 1.f);
		float magic = 1.f;
		//we can use Frame::tanTheta as thetaV is the angle betweeen surface normal n and wv
		float b = 1.f/(m_alpha * Frame::tanTheta(wv));
		if (b < 1.6f) 
		{
			magic = (3.535f*b + 2.181f*b*b) / (1 + 2.276f*b + 2.577f*b*b);
		}
		return xPlus(wv.dot(wh) / wv.dot(n))*magic;
	}
	float G(Vector3f wi, Vector3f wo, Vector3f wh) const
	{
		return G1(wi, wh)*G1(wo, wh);
	}

    /// Evaluate the BRDF for the given pair of directions
    Color3f eval(const BSDFQueryRecord &bRec) const {
		Vector3f wi = bRec.wi.normalized();
		Vector3f wo = bRec.wo.normalized();
		if (Frame::cosTheta(wi) < 0.f || Frame::cosTheta(wo) < 0.f)
		{
			return 0.f;
		}
		
		Vector3f wh = (wi + wo) / ((wi + wo).norm());

		float whCostTheta = Frame::cosTheta(wh);
		float woCosTheta = Frame::cosTheta(wo);
		float wiCosTheta = Frame::cosTheta(wi);
		float D = Warp::squareToBeckmannPdf(wh,m_alpha);
		return m_kd / M_PI + m_ks * (D*fresnel(wh.dot(wi), m_extIOR, m_intIOR)*G(wi, wo, wh)) / (4*wiCosTheta*woCosTheta*whCostTheta);
    }



    /// Evaluate the sampling density of \ref sample() wrt. solid angles
    float pdf(const BSDFQueryRecord &bRec) const {
		Vector3f wi = bRec.wi.normalized();
		Vector3f wo = bRec.wo.normalized();
		if (Frame::cosTheta(wi) < 0.f || Frame::cosTheta(wo) < 0.f)
		{
			return 0.f;
		}

		

		Vector3f wh = (wi + wo) / ((wi + wo).norm());
		float jh = 1.f/(4.f * (wh.dot(wo)));
		float woCosTheta = Frame::cosTheta(wo);
		float D = Warp::squareToBeckmannPdf(wh, m_alpha);

		return m_ks * D*jh + (1 - m_ks)*(woCosTheta / M_PI);
    }

    /// Sample the BRDF
    Color3f sample(BSDFQueryRecord &bRec, const Point2f &_sample) const {
		Vector3f wi = bRec.wi.normalized();
		if (Frame::cosTheta(wi) < 0.f)
		{
			return 0.f;
		}
		
		if (_sample[0] < m_ks)//in this case we sample specular part
		{
			//range of _sample[0] is [0,m_ks], need to get it back to [0,1]
			float scaledSample0 = _sample[0] / m_ks;
			//now we use squareToBeckmann to sample a normal
			Point2f scaledsamplePoint(scaledSample0, _sample[1]);
			Vector3f n = Warp::squareToBeckmann(scaledsamplePoint, m_alpha);
			n.normalize();//make sure than n is normalized
			//now we reflect wi using n in order to generate the outgoing direction wo
			bRec.wo = -wi + 2 * (n.dot(wi))*n;
		}
		else//in this case we sample the diffuse part
		{
			//range of _sample[0] is [m_ks,1], need to rescale to [0,1]
			float scaledSample0 = (_sample[0] - m_ks) / (1 - m_ks);
			Point2f scaledSamplePoint(scaledSample0, _sample[1]);
			bRec.wo = Warp::squareToCosineHemisphere(scaledSamplePoint);
		}
		Vector3f wo = bRec.wo.normalized();
		if (Frame::cosTheta(wo) < 0.f)
		{
			return 0.f;
		}

        // Note: Once you have implemented the part that computes the scattered
        // direction, the last part of this function should simply return the
        // BRDF value divided by the solid angle density and multiplied by the
        // cosine factor from the reflection equation, i.e.
         return eval(bRec) * Frame::cosTheta(wo) / pdf(bRec);
    }

    bool isDiffuse() const {
        /* While microfacet BRDFs are not perfectly diffuse, they can be
           handled by sampling techniques for diffuse/non-specular materials,
           hence we return true here */
        return true;
    }

    std::string toString() const {
        return tfm::format(
            "Microfacet[\n"
            "  alpha = %f,\n"
            "  intIOR = %f,\n"
            "  extIOR = %f,\n"
            "  kd = %s,\n"
            "  ks = %f\n"
            "]",
            m_alpha,
            m_intIOR,
            m_extIOR,
            m_kd.toString(),
            m_ks
        );
    }
private:
    float m_alpha;
    float m_intIOR, m_extIOR;
    float m_ks;
    Color3f m_kd;
};

NORI_REGISTER_CLASS(Microfacet, "microfacet");
NORI_NAMESPACE_END
