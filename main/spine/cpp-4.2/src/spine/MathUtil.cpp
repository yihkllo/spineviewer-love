
#include <spine/MathUtil.h>
#include <math.h>
#include <stdlib.h>
#include <cmath>

#ifdef _MSC_VER
#pragma warning(disable : 4723)
#endif

using namespace spine;

const float MathUtil::Pi = 3.1415926535897932385f;
const float MathUtil::Pi_2 = 3.1415926535897932385f * 2;
const float MathUtil::InvPi_2 = 1 / MathUtil::Pi_2;
const float MathUtil::Deg_Rad = (3.1415926535897932385f / 180.0f);
const float MathUtil::Rad_Deg = (180.0f / 3.1415926535897932385f);

float MathUtil::abs(float v) {
	return ((v) < 0 ? -(v) : (v));
}

float MathUtil::sign(float v) {
	return ((v) < 0 ? -1.0f : (v) > 0 ? 1.0f
									  : 0.0f);
}

float MathUtil::clamp(float x, float min, float max) {
	return ((x) < (min) ? (min) : ((x) > (max) ? (max) : (x)));
}

float MathUtil::fmod(float a, float b) {
	return (float) ::fmod(a, b);
}

float MathUtil::atan2(float y, float x) {
	return (float) ::atan2(y, x);
}

float MathUtil::atan2Deg(float y, float x) {
	return MathUtil::atan2(y, x) * MathUtil::Rad_Deg;
}

float MathUtil::cos(float radians) {
	return (float) ::cos(radians);
}

float MathUtil::sin(float radians) {
	return (float) ::sin(radians);
}

float MathUtil::sqrt(float v) {
	return (float) ::sqrt(v);
}

float MathUtil::acos(float v) {
	return (float) ::acos(v);
}

float MathUtil::sinDeg(float degrees) {
	return (float) ::sin(degrees * MathUtil::Deg_Rad);
}

float MathUtil::cosDeg(float degrees) {
	return (float) ::cos(degrees * MathUtil::Deg_Rad);
}

bool MathUtil::isNan(float v) {
	return std::isnan(v);
}

float MathUtil::quietNan() {
	return std::nan("");
}

float MathUtil::random() {
	return ::rand() / (float) RAND_MAX;
}

float MathUtil::randomTriangular(float min, float max) {
	return randomTriangular(min, max, (min + max) * 0.5f);
}

float MathUtil::randomTriangular(float min, float max, float mode) {
	float u = random();
	float d = max - min;
	if (u <= (mode - min) / d) return min + sqrt(u * d * (mode - min));
	return max - sqrt((1 - u) * d * (max - mode));
}

float MathUtil::pow(float a, float b) {
	return (float) ::pow(a, b);
}

float MathUtil::ceil(float v) {
	return ::ceil(v);
}
