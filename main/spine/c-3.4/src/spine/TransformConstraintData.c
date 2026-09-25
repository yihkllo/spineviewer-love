
#include <spine/TransformConstraintData.h>
#include <spine/extension.h>

spTransformConstraintData* spTransformConstraintData_create (const char* name) {
	spTransformConstraintData* self = NEW(spTransformConstraintData);
	MALLOC_STR(self->name, name);
	return self;
}

void spTransformConstraintData_dispose (spTransformConstraintData* self) {
	FREE(self->name);
	FREE(self->bones);
	FREE(self);
}
