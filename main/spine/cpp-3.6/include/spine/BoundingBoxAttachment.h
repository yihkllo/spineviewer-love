
#ifndef SPINE_BOUNDINGBOXATTACHMENT_H_
#define SPINE_BOUNDINGBOXATTACHMENT_H_

#include <spine/dll.h>
#include <spine/Attachment.h>
#include <spine/VertexAttachment.h>
#include <spine/Atlas.h>
#include <spine/Slot.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spBoundingBoxAttachment {
	spVertexAttachment super;
} spBoundingBoxAttachment;

SP_API spBoundingBoxAttachment* spBoundingBoxAttachment_create (const char* name);

#ifdef SPINE_SHORT_NAMES
typedef spBoundingBoxAttachment BoundingBoxAttachment;
#define BoundingBoxAttachment_create(...) spBoundingBoxAttachment_create(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
