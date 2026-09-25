
#ifndef SPINE_MESHATTACHMENT_H_
#define SPINE_MESHATTACHMENT_H_

#include <spine/Attachment.h>
#include <spine/Atlas.h>
#include <spine/Slot.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spMeshAttachment spMeshAttachment;
struct spMeshAttachment {
	spAttachment super;
	const char* path;

	int verticesCount;
	float* vertices;
	int hullLength;

	float* regionUVs;
	float* uvs;

	int trianglesCount;
	unsigned short* triangles;

	spMeshAttachment* const parentMesh;
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

spMeshAttachment* spMeshAttachment_create (const char* name);
void spMeshAttachment_updateUVs (spMeshAttachment* self);
void spMeshAttachment_computeWorldVertices (spMeshAttachment* self, spSlot* slot, float* worldVertices);
void spMeshAttachment_setParentMesh (spMeshAttachment* self, spMeshAttachment* parentMesh);

#ifdef SPINE_SHORT_NAMES
typedef spMeshAttachment MeshAttachment;
#define MeshAttachment_create(...) spMeshAttachment_create(__VA_ARGS__)
#define MeshAttachment_updateUVs(...) spMeshAttachment_updateUVs(__VA_ARGS__)
#define MeshAttachment_computeWorldVertices(...) spMeshAttachment_computeWorldVertices(__VA_ARGS__)
#define MeshAttachment_setParentMesh(...) spMeshAttachment_setParentMesh(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
