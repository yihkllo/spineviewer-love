
#include <spine/Event.h>
#include <spine/extension.h>

spEvent* spEvent_create (float time, spEventData* data) {
	spEvent* self = NEW(spEvent);
	CONST_CAST(spEventData*, self->data) = data;
	CONST_CAST(float, self->time) = time;
	return self;
}

void spEvent_dispose (spEvent* self) {
	FREE(self->stringValue);
	FREE(self);
}
