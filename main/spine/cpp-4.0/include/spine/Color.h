
#ifndef SPINE_COLOR_H
#define SPINE_COLOR_H

#include <spine/MathUtil.h>

namespace spine {
	class SP_API Color : public SpineObject {
	public:
		Color() : r(0), g(0), b(0), a(0) {
		}

		Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {
			clamp();
		}

		inline Color &set(float _r, float _g, float _b, float _a) {
			this->r = _r;
			this->g = _g;
			this->b = _b;
			this->a = _a;
			clamp();
			return *this;
		}

		inline Color &set(float _r, float _g, float _b) {
			this->r = _r;
			this->g = _g;
			this->b = _b;
			clamp();
			return *this;
		}

		inline Color &set(const Color &other) {
			r = other.r;
			g = other.g;
			b = other.b;
			a = other.a;
			clamp();
			return *this;
		}

		inline Color &add(float _r, float _g, float _b, float _a) {
			this->r += _r;
			this->g += _g;
			this->b += _b;
			this->a += _a;
			clamp();
			return *this;
		}

		inline Color &add(float _r, float _g, float _b) {
			this->r += _r;
			this->g += _g;
			this->b += _b;
			clamp();
			return *this;
		}

		inline Color &add(const Color &other) {
			r += other.r;
			g += other.g;
			b += other.b;
			a += other.a;
			clamp();
			return *this;
		}

		inline Color &clamp() {
			r = MathUtil::clamp(this->r, 0, 1);
			g = MathUtil::clamp(this->g, 0, 1);
			b = MathUtil::clamp(this->b, 0, 1);
			a = MathUtil::clamp(this->a, 0, 1);
			return *this;
		}

		float r, g, b, a;
	};
}


#endif
