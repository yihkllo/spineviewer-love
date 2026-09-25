
#ifndef Spine_MathUtil_h
#define Spine_MathUtil_h

#include <spine/SpineObject.h>

#include <string.h>

namespace spine {

	class SP_API MathUtil : public SpineObject {
	private:
		MathUtil();

	public:
		static const float Pi;
		static const float Pi_2;
		static const float Deg_Rad;
		static const float Rad_Deg;

		template<typename T>
		static inline T min(T a, T b) { return a < b ? a : b; }

		template<typename T>
		static inline T max(T a, T b) { return a > b ? a : b; }

		static float sign(float val);

		static float clamp(float x, float lower, float upper);

		static float abs(float v);

		static float sin(float radians);

		static float cos(float radians);

		static float sinDeg(float degrees);

		static float cosDeg(float degrees);

		static float atan2(float y, float x);

		static float acos(float v);

		static float sqrt(float v);

		static float fmod(float a, float b);

		static bool isNan(float v);

		static float random();

		static float randomTriangular(float min, float max);

		static float randomTriangular(float min, float max, float mode);

		static float pow(float a, float b);
	};

	struct SP_API Interpolation {
		virtual float apply(float a) = 0;

		virtual float interpolate(float start, float end, float a) {
			return start + (end - start) * apply(a);
		}

		virtual ~Interpolation() {};
	};

	struct SP_API PowInterpolation : public Interpolation {
		PowInterpolation(int power) : power(power) {
		}

		float apply(float a) {
			if (a <= 0.5f) return MathUtil::pow(a * 2.0f, (float) power) / 2.0f;
			return MathUtil::pow((a - 1.0f) * 2.0f, (float) power) / (power % 2 == 0 ? -2.0f : 2.0f) + 1.0f;
		}

		int power;
	};

	struct SP_API PowOutInterpolation : public Interpolation {
		PowOutInterpolation(int power) : power(power) {
		}

		float apply(float a) {
			return MathUtil::pow(a - 1, (float) power) * (power % 2 == 0 ? -1.0f : 1.0f) + 1.0f;
		}

		int power;
	};

}

#endif
