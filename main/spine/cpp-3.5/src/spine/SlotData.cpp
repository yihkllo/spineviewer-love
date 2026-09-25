
#include <spine/SlotData.h>
#include <spine/extension.h>

spSlotData* spSlotData_create (const int index, const char* name, spBoneData* boneData) {
	spSlotData* self = NEW(spSlotData);
	CONST_CAST(int, self->index) = index;
	MALLOC_STR(self->name, name);
	CONST_CAST(spBoneData*, self->boneData) = boneData;
	spColor_setFromFloats(&self->color, 1, 1, 1, 1);
	return self;
}

void spSlotData_dispose (spSlotData* self) {
	FREE(self->name);
	FREE(self->attachmentName);
	FREE(self->darkColor);
	FREE(self);
}

void spSlotData_setAttachmentName (spSlotData* self, const char* attachmentName) {
	FREE(self->attachmentName);
	if (attachmentName)
		MALLOC_STR(self->attachmentName, attachmentName);
	else
		CONST_CAST(char*, self->attachmentName) = 0;
}
