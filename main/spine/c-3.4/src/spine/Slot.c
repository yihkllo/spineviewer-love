
#include <spine/Slot.h>
#include <spine/extension.h>

typedef struct {
	spSlot super;
	float attachmentTime;
} _spSlot;

spSlot* spSlot_create (spSlotData* data, spBone* bone) {
	spSlot* self = SUPER(NEW(_spSlot));
	CONST_CAST(spSlotData*, self->data) = data;
	CONST_CAST(spBone*, self->bone) = bone;
	spSlot_setToSetupPose(self);
	return self;
}

void spSlot_dispose (spSlot* self) {
	FREE(self->attachmentVertices);
	FREE(self);
}

void spSlot_setAttachment (spSlot* self, spAttachment* attachment) {
	if (attachment == self->attachment) return;
	CONST_CAST(spAttachment*, self->attachment) = attachment;
	SUB_CAST(_spSlot, self)->attachmentTime = self->bone->skeleton->time;
	self->attachmentVerticesCount = 0;
}

void spSlot_setAttachmentTime (spSlot* self, float time) {
	SUB_CAST(_spSlot, self)->attachmentTime = self->bone->skeleton->time - time;
}

float spSlot_getAttachmentTime (const spSlot* self) {
	return self->bone->skeleton->time - SUB_CAST(_spSlot, self) ->attachmentTime;
}

void spSlot_setToSetupPose (spSlot* self) {
	self->r = self->data->r;
	self->g = self->data->g;
	self->b = self->data->b;
	self->a = self->data->a;

	if (!self->data->attachmentName)
		spSlot_setAttachment(self, 0);
	else {
		spAttachment* attachment = spSkeleton_getAttachmentForSlotIndex(
				self->bone->skeleton, self->data->index, self->data->attachmentName);
		CONST_CAST(spAttachment*, self->attachment) = 0;
		spSlot_setAttachment(self, attachment);
	}
}
