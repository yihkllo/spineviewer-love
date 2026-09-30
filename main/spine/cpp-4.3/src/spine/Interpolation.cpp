#include <spine/Interpolation.h>
#include <spine/MathUtil.h>

using namespace spine;

namespace {
	class LinearInterpolation : public Interpolation {
	public:
		float apply(float a) override {
			return a;
		}
	};

	class SmoothInterpolation : public Interpolation {
	public:
		float apply(float a) override {
			return a * a * (3 - 2 * a);
		}
	};

	class SlowFastInterpolation : public Interpolation {
	public:
		float apply(float a) override {
			return a * a;
		}
	};

	class FastSlowInterpolation : public Interpolation {
	public:
		float apply(float a) override {
			return (a - 1) * (a - 1) * -1 + 1;
		}
	};

	class CircleInterpolation : public Interpolation {
	public:
		float apply(float a) override {
			if (a <= 0.5f) {
				a *= 2;
				return (1 - MathUtil::sqrt(1 - a * a)) / 2;
			}
			a--;
			a *= 2;
			return (MathUtil::sqrt(1 - a * a) + 1) / 2;
		}
	};
}

Interpolation::~Interpolation() {
}

float Interpolation::apply(float a) {
	return a;
}

float Interpolation::apply(float start, float end, float a) {
	return start + (end - start) * apply(a);
}

Interpolation &Interpolation::linear() {
	static LinearInterpolation interpolation;
	return interpolation;
}

Interpolation &Interpolation::smooth() {
	static SmoothInterpolation interpolation;
	return interpolation;
}

Interpolation &Interpolation::slowFast() {
	static SlowFastInterpolation interpolation;
	return interpolation;
}

Interpolation &Interpolation::fastSlow() {
	static FastSlowInterpolation interpolation;
	return interpolation;
}

Interpolation &Interpolation::circle() {
	static CircleInterpolation interpolation;
	return interpolation;
}
