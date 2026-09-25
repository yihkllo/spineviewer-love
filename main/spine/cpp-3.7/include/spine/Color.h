
#ifndef SPINE_COLOR_H_
#define SPINE_COLOR_H_

#include <spine/dll.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spColor {
	float r, g, b, a;

#ifdef __cplusplus
	spColor() :
		r(0), g(0), b(0), a(0) {
	}

	bool operator==(const spColor& rhs) {
		return r == rhs.r && g == rhs.g && b == rhs.b && a == rhs.a;
	}
#endif
} spColor;

SP_API spColor* spColor_create();
SP_API void spColor_dispose(spColor* self);
SP_API void spColor_setFromFloats(spColor* color, float r, float g, float b, float a);
SP_API void spColor_setFromColor(spColor* color, spColor* otherColor);
SP_API void spColor_addFloats(spColor* color, float r, float g, float b, float a);
SP_API void spColor_addColor(spColor* color, spColor* otherColor);
SP_API void spColor_clamp(spColor* color);

#ifdef SPINE_SHORT_NAMES
typedef spColor color;
#define Color_create() spColor_create()
#define Color_dispose(...) spColor_dispose(__VA_ARGS__)
#define Color_setFromFloats(...) spColor_setFromFloats(__VA_ARGS__)
#define Color_setFromColor(...) spColor_setFromColor(__VA_ARGS__)
#define Color_addColor(...) spColor_addColor(__VA_ARGS__)
#define Color_addFloats(...) spColor_addFloats(__VA_ARGS__)
#define Color_clamp(...) spColor_clamp(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
