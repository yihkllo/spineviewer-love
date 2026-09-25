
#include <spine/BoneData.h>
#include <spine/extension.h>

spBoneData* spBoneData_create (const char* name, spBoneData* parent) {
	spBoneData* self = NEW(spBoneData);
	MALLOC_STR(self->name, name);
	CONST_CAST(spBoneData*, self->parent) = parent;
	self->scaleX = 1;
	self->scaleY = 1;
	self->inheritScale = 1;
	self->inheritRotation = 1;
	return self;
}

void spBoneData_dispose (spBoneData* self) {
	FREE(self->name);
	FREE(self);
}
