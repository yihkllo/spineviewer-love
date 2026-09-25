
#include <spine/EventData.h>
#include <spine/extension.h>

spEventData* spEventData_create (const char* name) {
	spEventData* self = NEW(spEventData);
	MALLOC_STR(self->name, name);
	return self;
}

void spEventData_dispose (spEventData* self) {
	FREE(self->audioPath);
	FREE(self->stringValue);
	FREE(self->name);
	FREE(self);
}
