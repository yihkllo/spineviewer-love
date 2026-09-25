
#include <spine/VertexEffect.h>
#include <spine/extension.h>

void _spJitterVertexEffect_begin(spVertexEffect* self, spSkeleton* skeleton) {
}

void _spJitterVertexEffect_transform(spVertexEffect* self, float* x, float* y, float* u, float* v, spColor* light, spColor* dark) {
	spJitterVertexEffect* internal = (spJitterVertexEffect*)self;
	float jitterX = internal->jitterX;
	float jitterY = internal->jitterY;
	(*x) += _spMath_randomTriangular(-jitterX, jitterY);
	(*y) += _spMath_randomTriangular(-jitterX, jitterY);
}

void _spJitterVertexEffect_end(spVertexEffect* self) {
}

spJitterVertexEffect* spJitterVertexEffect_create(float jitterX, float jitterY) {
	spJitterVertexEffect* effect = CALLOC(spJitterVertexEffect, 1);
	effect->super.begin = _spJitterVertexEffect_begin;
	effect->super.transform = _spJitterVertexEffect_transform;
	effect->super.end = _spJitterVertexEffect_end;
	effect->jitterX = jitterX;
	effect->jitterY = jitterY;
	return effect;
}

void spJitterVertexEffect_dispose(spJitterVertexEffect* effect) {
	FREE(effect);
}

void _spSwirlVertexEffect_begin(spVertexEffect* self, spSkeleton* skeleton) {
	spSwirlVertexEffect* internal = (spSwirlVertexEffect*)self;
	internal->worldX = skeleton->x + internal->centerX;
	internal->worldY = skeleton->y + internal->centerY;
}

void _spSwirlVertexEffect_transform(spVertexEffect* self, float* positionX, float* positionY, float* u, float* v, spColor* light, spColor* dark) {
	spSwirlVertexEffect* internal = (spSwirlVertexEffect*)self;
	float radAngle = internal->angle * DEG_RAD;
	float x = *positionX - internal->worldX;
	float y = *positionY - internal->worldY;
	float dist = SQRT(x * x + y * y);
	if (dist < internal->radius) {
		float theta = _spMath_interpolate(_spMath_pow2_apply, 0, radAngle, (internal->radius - dist) / internal->radius);
		float cosine = COS(theta);
		float sine = SIN(theta);
		(*positionX) = cosine * x - sine * y + internal->worldX;
		(*positionY) = sine * x + cosine * y + internal->worldY;
	}
}

void _spSwirlVertexEffect_end(spVertexEffect* self) {
}

spSwirlVertexEffect* spSwirlVertexEffect_create(float radius) {
	spSwirlVertexEffect* effect = CALLOC(spSwirlVertexEffect, 1);
	effect->super.begin = _spSwirlVertexEffect_begin;
	effect->super.transform = _spSwirlVertexEffect_transform;
	effect->super.end = _spSwirlVertexEffect_end;
	effect->radius = radius;
	return effect;
}

void spSwirlVertexEffect_dispose(spSwirlVertexEffect* effect) {
	FREE(effect);
}

