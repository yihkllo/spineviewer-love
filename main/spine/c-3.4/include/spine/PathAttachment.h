
#ifndef SPINE_PATHATTACHMENT_H_
#define SPINE_PATHATTACHMENT_H_

#include <spine/Attachment.h>
#include <spine/VertexAttachment.h>
#include <spine/Atlas.h>
#include <spine/Slot.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spPathAttachment {
	spVertexAttachment super;
	int lengthsLength;
	float* lengths;
	int  closed, constantSpeed;
} spPathAttachment;

spPathAttachment* spPathAttachment_create (const char* name);
void spPathAttachment_computeWorldVertices (spPathAttachment* self, spSlot* slot, float* worldVertices);
void spPathAttachment_computeWorldVertices1 (spPathAttachment* self, spSlot* slot, int start, int count, float* worldVertices, int offset);

#ifdef SPINE_SHORT_NAMES
typedef spPathAttachment PathAttachment;
#define PathAttachment_create(...) spPathAttachment_create(__VA_ARGS__)
#define PathAttachment_computeWorldVertices(...) spPathAttachment_computeWorldVertices(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
