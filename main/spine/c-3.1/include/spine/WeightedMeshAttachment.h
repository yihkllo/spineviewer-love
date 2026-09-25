
#ifndef SPINE_WEIGHTEDMESHATTACHMENT_H_
#define SPINE_WEIGHTEDMESHATTACHMENT_H_

#include <spine/Attachment.h>
#include <spine/Slot.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spWeightedMeshAttachment spWeightedMeshAttachment;
struct spWeightedMeshAttachment {
	spAttachment super;
	const char* path;

	int bonesCount;
	int* bones;

	int weightsCount;
	float* weights;

	int trianglesCount;
	unsigned short* triangles;

	int uvsCount;
	float* regionUVs;
	float* uvs;
	int hullLength;

	spWeightedMeshAttachment* const parentMesh;
	int inheritFFD;

	float r, g, b, a;

	void* rendererObject;
	int regionOffsetX, regionOffsetY;
	int regionWidth, regionHeight;
	int regionOriginalWidth, regionOriginalHeight;
	float regionU, regionV, regionU2, regionV2;
	int regionRotate;

	int edgesCount;
	int* edges;
	float width, height;
};

spWeightedMeshAttachment* spWeightedMeshAttachment_create (const char* name);
void spWeightedMeshAttachment_updateUVs (spWeightedMeshAttachment* self);
void spWeightedMeshAttachment_computeWorldVertices (spWeightedMeshAttachment* self, spSlot* slot, float* worldVertices);
void spWeightedMeshAttachment_setParentMesh (spWeightedMeshAttachment* self, spWeightedMeshAttachment* parentMesh);

#ifdef SPINE_SHORT_NAMES
typedef spWeightedMeshAttachment WeightedMeshAttachment;
#define WeightedMeshAttachment_create(...) spWeightedMeshAttachment_create(__VA_ARGS__)
#define WeightedMeshAttachment_updateUVs(...) spWeightedMeshAttachment_updateUVs(__VA_ARGS__)
#define WeightedMeshAttachment_computeWorldVertices(...) spWeightedMeshAttachment_computeWorldVertices(__VA_ARGS__)
#define WeightedMeshAttachment_setParentMesh(...) spWeightedMeshAttachment_setParentMesh(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
