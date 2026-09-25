
#ifndef SPINE_VERTEXEFFECT_H_
#define SPINE_VERTEXEFFECT_H_

#include <spine/dll.h>
#include <spine/Skeleton.h>
#include <spine/Color.h>

#ifdef __cplusplus
extern "C" {
#endif

struct spVertexEffect;

typedef void (*spVertexEffectBegin)(struct spVertexEffect *self, spSkeleton *skeleton);

typedef void (*spVertexEffectTransform)(struct spVertexEffect *self, float *x, float *y, float *u, float *v,
										spColor *light, spColor *dark);

typedef void (*spVertexEffectEnd)(struct spVertexEffect *self);

typedef struct spVertexEffect {
	spVertexEffectBegin begin;
	spVertexEffectTransform transform;
	spVertexEffectEnd end;
} spVertexEffect;

typedef struct spJitterVertexEffect {
	spVertexEffect super;
	float jitterX;
	float jitterY;
} spJitterVertexEffect;

typedef struct spSwirlVertexEffect {
	spVertexEffect super;
	float centerX;
	float centerY;
	float radius;
	float angle;
	float worldX;
	float worldY;
} spSwirlVertexEffect;

SP_API spJitterVertexEffect *spJitterVertexEffect_create(float jitterX, float jitterY);

SP_API void spJitterVertexEffect_dispose(spJitterVertexEffect *effect);

SP_API spSwirlVertexEffect *spSwirlVertexEffect_create(float radius);

SP_API void spSwirlVertexEffect_dispose(spSwirlVertexEffect *effect);

#ifdef __cplusplus
}
#endif

#endif
