
#ifndef SPINE_POINTATTACHMENT_H_
#define SPINE_POINTATTACHMENT_H_

#include <spine/dll.h>
#include <spine/Attachment.h>
#include <spine/VertexAttachment.h>
#include <spine/Atlas.h>
#include <spine/Slot.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spPointAttachment {
	spVertexAttachment super;
	float x, y, rotation;
	spColor color;
} spPointAttachment;

SP_API spPointAttachment* spPointAttachment_create (const char* name);
SP_API void spPointAttachment_computeWorldPosition (spPointAttachment* self, spBone* bone, float* x, float* y);
SP_API float spPointAttachment_computeWorldRotation (spPointAttachment* self, spBone* bone);

#ifdef SPINE_SHORT_NAMES
typedef spPointAttachment PointAttachment;
#define PointAttachment_create(...) spPointAttachment_create(__VA_ARGS__)
#define PointAttachment_computeWorldPosition(...) spPointAttachment_computeWorldPosition(__VA_ARGS__)
#define PointAttachment_computeWorldRotation(...) spPointAttachment_computeWorldRotation(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
