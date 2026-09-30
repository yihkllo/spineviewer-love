#ifndef SPINE_COLOR_H
#define SPINE_COLOR_H

#include <spine/MathUtil.h>
#include <string.h>
#include <stdlib.h>

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

		static Color valueOf(const char *hexString) {
			Color color;
			valueOf(hexString, color);
			return color;
		}

		static void valueOf(const char *hexString, Color &color) {
			size_t len = strlen(hexString);
			if (len >= 6) {
				color.r = parseHex(hexString, 0);
				color.g = parseHex(hexString, 1);
				color.b = parseHex(hexString, 2);
				color.a = len >= 8 ? parseHex(hexString, 3) : 1.0f;
			}
		}

		static float parseHex(const char *value, size_t index) {
			char digits[3];
			digits[0] = value[index * 2];
			digits[1] = value[index * 2 + 1];
			digits[2] = '\0';
			return strtoul(digits, NULL, 16) / 255.0f;
		}

		static void rgba8888ToColor(Color &color, int value) {
			unsigned int rgba = (unsigned int) value;
			color.r = ((rgba & 0xff000000) >> 24) / 255.0f;
			color.g = ((rgba & 0x00ff0000) >> 16) / 255.0f;
			color.b = ((rgba & 0x0000ff00) >> 8) / 255.0f;
			color.a = (rgba & 0x000000ff) / 255.0f;
		}

		static void rgb888ToColor(Color &color, int value) {
			unsigned int rgb = (unsigned int) value;
			color.r = ((rgb & 0xff0000) >> 16) / 255.0f;
			color.g = ((rgb & 0x00ff00) >> 8) / 255.0f;
			color.b = (rgb & 0x0000ff) / 255.0f;
			color.a = 1.0f;
		}

		float r, g, b, a;
	};
}

#endif
