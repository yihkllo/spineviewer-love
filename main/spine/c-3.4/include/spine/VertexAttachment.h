
#ifndef SPINE_VERTEXATTACHMENT_H_
#define SPINE_VERTEXATTACHMENT_H_

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
};

void spVertexAttachment_computeWorldVertices (spVertexAttachment* self, spSlot* slot, float* worldVertices);
void spVertexAttachment_computeWorldVertices1 (spVertexAttachment* self, int start, int count, spSlot* slot, float* worldVertices, int offset);

#ifdef SPINE_SHORT_NAMES
typedef spVertexAttachment VertexAttachment;
#define VertexAttachment_computeWorldVertices(...) spVertexAttachment_computeWorldVertices(__VA_ARGS__)
#define VertexAttachment_computeWorldVertices1(...) spVertexAttachment_computeWorldVertices1(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
