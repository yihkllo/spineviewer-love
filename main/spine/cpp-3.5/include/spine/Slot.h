
#ifndef SPINE_SLOT_H_
#define SPINE_SLOT_H_

#include <spine/Bone.h>
#include <spine/Attachment.h>
#include <spine/SlotData.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spSlot {
	spSlotData* const data;
	spBone* const bone;
	spColor color;
	spColor* darkColor;
	spAttachment* const attachment;

	int attachmentVerticesCapacity;
	int attachmentVerticesCount;
	float* attachmentVertices;

#ifdef __cplusplus
	spSlot() :
		data(0),
		bone(0),
		color(),
		darkColor(0),
		attachment(0),
		attachmentVerticesCapacity(0),
		attachmentVerticesCount(0),
		attachmentVertices(0) {
	}
#endif
} spSlot;

spSlot* spSlot_create (spSlotData* data, spBone* bone);
void spSlot_dispose (spSlot* self);

void spSlot_setAttachment (spSlot* self, spAttachment* attachment);

void spSlot_setAttachmentTime (spSlot* self, float time);
float spSlot_getAttachmentTime (const spSlot* self);

void spSlot_setToSetupPose (spSlot* self);

#ifdef SPINE_SHORT_NAMES
typedef spSlot Slot;
#define Slot_create(...) spSlot_create(__VA_ARGS__)
#define Slot_dispose(...) spSlot_dispose(__VA_ARGS__)
#define Slot_setAttachment(...) spSlot_setAttachment(__VA_ARGS__)
#define Slot_setAttachmentTime(...) spSlot_setAttachmentTime(__VA_ARGS__)
#define Slot_getAttachmentTime(...) spSlot_getAttachmentTime(__VA_ARGS__)
#define Slot_setToSetupPose(...) spSlot_setToSetupPose(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
