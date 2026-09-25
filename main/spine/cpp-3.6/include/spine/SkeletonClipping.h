
#ifndef SPINE_SKELETONCLIPPING_H
#define SPINE_SKELETONCLIPPING_H

#include <spine/dll.h>
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

SP_API spSkeletonClipping* spSkeletonClipping_create();
SP_API int spSkeletonClipping_clipStart(spSkeletonClipping* self, spSlot* slot, spClippingAttachment* clip);
SP_API void spSkeletonClipping_clipEnd(spSkeletonClipping* self, spSlot* slot);
SP_API void spSkeletonClipping_clipEnd2(spSkeletonClipping* self);
SP_API int   spSkeletonClipping_isClipping(spSkeletonClipping* self);
SP_API void spSkeletonClipping_clipTriangles(spSkeletonClipping* self, float* vertices, int verticesLength, unsigned short* triangles, int trianglesLength, float* uvs, int stride);
SP_API void spSkeletonClipping_dispose(spSkeletonClipping* self);

#ifdef __cplusplus
}
#endif

#endif
