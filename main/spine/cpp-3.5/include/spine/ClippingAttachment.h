
#ifndef SPINE_CLIPPINGATTACHMENT_H_
#define SPINE_CLIPPINGATTACHMENT_H_

#include <spine/Attachment.h>
#include <spine/VertexAttachment.h>
#include <spine/Atlas.h>
#include <spine/Slot.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spClippingAttachment {
	spVertexAttachment super;
	spSlotData* endSlot;
} spClippingAttachment;

void _spClippingAttachment_dispose(spAttachment* self);
spClippingAttachment* spClippingAttachment_create (const char* name);

#ifdef SPINE_SHORT_NAMES
typedef spClippingAttachment ClippingAttachment;
#define ClippingAttachment_create(...) spClippingAttachment_create(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
