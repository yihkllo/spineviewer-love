#ifndef Spine_MathUtil_h
#define Spine_MathUtil_h

#include <spine/SpineObject.h>

#include <string.h>

#undef min
#undef max

namespace spine {

	class SP_API MathUtil : public SpineObject {
	private:
		MathUtil();

	public:
		static const float Epsilon;
		static const float EpsilonSq;
		static const float Pi;
		static const float Pi_2;
		static const float InvPi_2;
		static const float Deg_Rad;
		static const float Rad_Deg;

		template<typename T>
		static inline T min(T a, T b) {
			return a < b ? a : b;
		}

		template<typename T>
		static inline T max(T a, T b) {
			return a > b ? a : b;
		}

		static float sign(float val);

		static float clamp(float x, float lower, float upper);

		static float abs(float v);

		static float sin(float radians);

		static float cos(float radians);

		static float sinDeg(float degrees);

		static float cosDeg(float degrees);

		static float atan2(float y, float x);

		static float atan2Deg(float x, float y);

		static float acos(float v);

		static float sqrt(float v);

		static float fmod(float a, float b);

		static bool isNan(float v);

		static float quietNan();

		static float random();

		static float randomTriangular(float min, float max);

		static float randomTriangular(float min, float max, float mode);

		static float pow(float a, float b);

		static float ceil(float v);
	};
}

#endif
