
#include <spine/Color.h>
#include <spine/extension.h>

spColor* spColor_create() {
	return MALLOC(spColor, 1);
}

void spColor_dispose(spColor* self) {
	if (self) FREE(self);
}

void spColor_setFromFloats(spColor* self, float r, float g, float b, float a) {
	self->r = r;
	self->g = g;
	self->b = b;
	self->a = a;
}

void spColor_setFromColor(spColor* self, spColor* otherColor) {
	self->r = otherColor->r;
	self->g = otherColor->g;
	self->b = otherColor->b;
	self->a = otherColor->a;
}

void spColor_addColor(spColor* self, spColor* otherColor) {
	self->r += otherColor->r;
	self->g += otherColor->g;
	self->b += otherColor->b;
	self->a += otherColor->a;
	spColor_clamp(self);
}

void spColor_addFloats(spColor* self, float r, float g, float b, float a) {
	self->r += r;
	self->g += g;
	self->b += b;
	self->a += a;
	spColor_clamp(self);
}

void spColor_clamp(spColor* self) {
	if (self->r < 0) self->r = 0;
	else if (self->r > 1) self->r = 1;

	if (self->g < 0) self->g = 0;
	else if (self->g > 1) self->g = 1;

	if (self->b < 0) self->b = 0;
	else if (self->b > 1) self->b = 1;

	if (self->a < 0) self->a = 0;
	else if (self->a > 1) self->a = 1;
}
