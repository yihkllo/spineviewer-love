
#ifndef SPINE_VERTEXATTACHMENT_H_
#define SPINE_VERTEXATTACHMENT_H_

#include <spine/dll.h>
#include <spine/Attachment.h>
#include <spine/Slot.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spVertexAttachment spVertexAttachment;
struct spVertexAttachment {
	spAttachment super;

	int bonesCount;
	int* bones;

	int verticesCount;
	float* vertices;

	int worldVerticesLength;

	int id;
};

SP_API void spVertexAttachment_computeWorldVertices (spVertexAttachment* self, spSlot* slot, int start, int count, float* worldVertices, int offset, int stride);

#ifdef SPINE_SHORT_NAMES
typedef spVertexAttachment VertexAttachment;
#define VertexAttachment_computeWorldVertices(...) spVertexAttachment_computeWorldVertices(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
