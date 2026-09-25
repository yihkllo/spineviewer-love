
#ifndef SPINE_REGIONATTACHMENT_H_
#define SPINE_REGIONATTACHMENT_H_

#include <spine/Attachment.h>
#include <spine/Atlas.h>
#include <spine/Slot.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	SP_VERTEX_X1 = 0, SP_VERTEX_Y1, SP_VERTEX_X2, SP_VERTEX_Y2, SP_VERTEX_X3, SP_VERTEX_Y3, SP_VERTEX_X4, SP_VERTEX_Y4
} spVertexIndex;

typedef struct spRegionAttachment {
	spAttachment super;
	const char* path;
	float x, y, scaleX, scaleY, rotation, width, height;
	float r, g, b, a;

	void* rendererObject;
	int regionOffsetX, regionOffsetY;
	int regionWidth, regionHeight;
	int regionOriginalWidth, regionOriginalHeight;

	float offset[8];
	float uvs[8];
} spRegionAttachment;

spRegionAttachment* spRegionAttachment_create (const char* name);
void spRegionAttachment_setUVs (spRegionAttachment* self, float u, float v, float u2, float v2, int rotate);
void spRegionAttachment_updateOffset (spRegionAttachment* self);
void spRegionAttachment_computeWorldVertices (spRegionAttachment* self, spBone* bone, float* vertices);

#ifdef SPINE_SHORT_NAMES
typedef spVertexIndex VertexIndex;
#define VERTEX_X1 SP_VERTEX_X1
#define VERTEX_Y1 SP_VERTEX_Y1
#define VERTEX_X2 SP_VERTEX_X2
#define VERTEX_Y2 SP_VERTEX_Y2
#define VERTEX_X3 SP_VERTEX_X3
#define VERTEX_Y3 SP_VERTEX_Y3
#define VERTEX_X4 SP_VERTEX_X4
#define VERTEX_Y4 SP_VERTEX_Y4
typedef spRegionAttachment RegionAttachment;
#define RegionAttachment_create(...) spRegionAttachment_create(__VA_ARGS__)
#define RegionAttachment_setUVs(...) spRegionAttachment_setUVs(__VA_ARGS__)
#define RegionAttachment_updateOffset(...) spRegionAttachment_updateOffset(__VA_ARGS__)
#define RegionAttachment_computeWorldVertices(...) spRegionAttachment_computeWorldVertices(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
