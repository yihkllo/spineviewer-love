
#ifndef SPINE_MESHATTACHMENT_H_
#define SPINE_MESHATTACHMENT_H_

#include <spine/dll.h>
#include <spine/Attachment.h>
#include <spine/VertexAttachment.h>
#include <spine/Atlas.h>
#include <spine/Slot.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spMeshAttachment spMeshAttachment;
struct spMeshAttachment {
	spVertexAttachment super;

	void* rendererObject;
	int regionOffsetX, regionOffsetY;
	int regionWidth, regionHeight;
	int regionOriginalWidth, regionOriginalHeight;
	float regionU, regionV, regionU2, regionV2;
	int regionRotate;

	const char* path;

	float* regionUVs;
	float* uvs;

	int trianglesCount;
	unsigned short* triangles;

	spColor color;

	int hullLength;

	spMeshAttachment* const parentMesh;
	int inheritDeform;

	int edgesCount;
	int* edges;
	float width, height;
};

SP_API spMeshAttachment* spMeshAttachment_create (const char* name);
SP_API void spMeshAttachment_updateUVs (spMeshAttachment* self);
SP_API void spMeshAttachment_setParentMesh (spMeshAttachment* self, spMeshAttachment* parentMesh);

#ifdef SPINE_SHORT_NAMES
typedef spMeshAttachment MeshAttachment;
#define MeshAttachment_create(...) spMeshAttachment_create(__VA_ARGS__)
#define MeshAttachment_updateUVs(...) spMeshAttachment_updateUVs(__VA_ARGS__)
#define MeshAttachment_setParentMesh(...) spMeshAttachment_setParentMesh(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
