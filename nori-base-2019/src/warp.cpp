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

#include <nori/warp.h>
#include <nori/vector.h>
#include <nori/frame.h>

NORI_NAMESPACE_BEGIN

Point2f Warp::squareToUniformSquare(const Point2f &sample) {
    return sample;
}

float Warp::squareToUniformSquarePdf(const Point2f &sample) {
    return ((sample.array() >= 0).all() && (sample.array() <= 1).all()) ? 1.0f : 0.0f;
}

float tentSample(float sample)
{
	float res;
	if (sample <= 0.5f)
	{
		res = -1 + std::sqrt(2 * sample);
	}
	else {
		res = 1 - std::sqrt(2 - 2 * sample);
	}
	return res;
}

Point2f Warp::squareToTent(const Point2f &sample) {
	float warpedX, warpedY;
	warpedX = tentSample(sample[0]);
	warpedY = tentSample(sample[1]);

	return Point2f(warpedX, warpedY);
}

float Warp::squareToTentPdf(const Point2f &p) {
	float pdfX = ( (p[0] >= -1.f) && (p[0] <= 1.f) ) ? (1.f - std::abs(p[0])) : 0;
	float pdfY = ((p[1] >= -1.f) && (p[1] <= 1.f)) ? (1.f - std::abs(p[1])) : 0;
	return pdfX*pdfY;
}

Point2f Warp::squareToUniformDisk(const Point2f &sample) {
	float r = std::sqrt(sample[0]);
	float theta = 2 * M_PI * sample[1];
	return Point2f(r * std::cos(theta), r * std::sin(theta));
}

float Warp::squareToUniformDiskPdf(const Point2f &p) {
	return (p[0] * p[0] + p[1] * p[1] < 1) ? INV_PI : 0;
}

Vector3f Warp::squareToUniformSphere(const Point2f &sample) {
	float z = 1 - 2 * sample[0];
	float val = std::sqrt(std::max((float)0, (float)1 - z * z));
	float theta = 2 * M_PI*sample[1];
	return Vector3f(val*std::cosf(theta), val*std::sinf(theta), z);
}

float Warp::squareToUniformSpherePdf(const Vector3f &v) {
	float val = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	return (((val) <= (1 + 0.0003f)) && (val >= (1 - 0.0003f))) ? INV_FOURPI : 0.0f;
}

Vector3f Warp::squareToUniformHemisphere(const Point2f &sample) {
	float val = std::sqrt(std::max((float)0,(float)1. - sample[0] * sample[0]));
	float theta = 2 * M_PI*sample[1];
	return Vector3f(std::cosf(theta)*val, std::sinf(theta)*val, sample[0]);
}

float Warp::squareToUniformHemispherePdf(const Vector3f &v) {
	float val = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	return ( (((val) <= (1 + 0.0003f)) && (val >= (1 - 0.0003f))) && v[2]>=0) ? INV_TWOPI : 0;
}

Vector3f Warp::squareToCosineHemisphere(const Point2f &sample) {
	Point2f pDisk = squareToUniformDisk(sample);
	float z = std::sqrt(std::max(0.0f, 1.0f - pDisk[0] * pDisk[0] - pDisk[1] * pDisk[1]));
	return Vector3f(pDisk[0], pDisk[1], z);
}

float Warp::squareToCosineHemispherePdf(const Vector3f &v) {
	float val = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	return ((((val) <= (1 + 0.0003f)) && (val >= (1 - 0.0003f))) && v[2] >= 0) ? (v[2] * INV_PI) : 0;
}

Vector3f Warp::squareToBeckmann(const Point2f &sample, float alpha) {
	float phi = 2 * M_PI*sample[0];
	float tan2Theta = -alpha * alpha * std::log(1 - sample[1]);
	//expression for cosTheta was found using tan2t = sin2t/cos2t and sin2t+cos2t = 1
	float cosTheta = 1 / std::sqrtf(tan2Theta + 1);
	float sinTheta = std::sqrtf(1 - cosTheta * cosTheta);

	float x = sinTheta * cosf(phi);
	float y = sinTheta * sinf(phi);
	float z = cosTheta;
	return Vector3f(x, y, z);
}

float Warp::squareToBeckmannPdf(const Vector3f &m, float alpha) {
	float tanTheta = Frame::tanTheta(m);
	float tan2Theta = tanTheta * tanTheta;
	if (std::isinf(tan2Theta)) { return 0.f; }
	float cosTheta = Frame::cosTheta(m);
	if (cosTheta < 0.0f)
	{
		return 0.f;
	}
	float cos3Theta = cosTheta * cosTheta * cosTheta;
	
	return std::expf(-tan2Theta / (alpha*alpha)) / (M_PI*alpha*alpha*cos3Theta);
}

NORI_NAMESPACE_END
