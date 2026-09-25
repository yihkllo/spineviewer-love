
#ifndef SPINE_SKELETONCLIPPING_H
#define SPINE_SKELETONCLIPPING_H

#include <spine/Array.h>
#include <spine/ClippingAttachment.h>
#include <spine/Slot.h>
#include <spine/Triangulator.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spSkeletonClipping {
	spTriangulator* triangulator;
	spFloatArray* clippingPolygon;
	spFloatArray* clipOutput;
	spFloatArray* clippedVertices;
	spFloatArray* clippedUVs;
	spUnsignedShortArray* clippedTriangles;
	spFloatArray* scratch;
	spClippingAttachment* clipAttachment;
	spArrayFloatArray* clippingPolygons;
} spSkeletonClipping;

spSkeletonClipping* spSkeletonClipping_create();
int spSkeletonClipping_clipStart(spSkeletonClipping* self, spSlot* slot, spClippingAttachment* clip);
void spSkeletonClipping_clipEnd(spSkeletonClipping* self, spSlot* slot);
void spSkeletonClipping_clipEnd2(spSkeletonClipping* self);
int   spSkeletonClipping_isClipping(spSkeletonClipping* self);
void spSkeletonClipping_clipTriangles(spSkeletonClipping* self, float* vertices, int verticesLength, unsigned short* triangles, int trianglesLength, float* uvs, int stride);
void spSkeletonClipping_dispose(spSkeletonClipping* self);

#ifdef __cplusplus
}
#endif

#endif
