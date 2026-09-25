
#include <spine/IkConstraintData.h>
#include <spine/extension.h>

spIkConstraintData* spIkConstraintData_create (const char* name) {
	spIkConstraintData* self = NEW(spIkConstraintData);
	MALLOC_STR(self->name, name);
	self->bendDirection = 1;
	self->mix = 1;
	return self;
}

void spIkConstraintData_dispose (spIkConstraintData* self) {
	FREE(self->name);
	FREE(self->bones);
	FREE(self);
}
