
#include <spine/PathConstraintData.h>
#include <spine/extension.h>

spPathConstraintData* spPathConstraintData_create (const char* name) {
	spPathConstraintData* self = NEW(spPathConstraintData);
	MALLOC_STR(self->name, name);
	return self;
}

void spPathConstraintData_dispose (spPathConstraintData* self) {
	FREE(self->name);
	FREE(self->bones);
	FREE(self);
}
