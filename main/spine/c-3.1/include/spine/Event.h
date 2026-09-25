
#ifndef SPINE_EVENT_H_
#define SPINE_EVENT_H_

#include <spine/EventData.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spEvent {
	spEventData* const data;
	float const time;
	int intValue;
	float floatValue;
	const char* stringValue;

#ifdef __cplusplus
	spEvent() :
		data(0),
		time(0),
		intValue(0),
		floatValue(0),
		stringValue(0) {
	}
#endif
} spEvent;

spEvent* spEvent_create (float time, spEventData* data);
void spEvent_dispose (spEvent* self);

#ifdef SPINE_SHORT_NAMES
typedef spEvent Event;
#define Event_create(...) spEvent_create(__VA_ARGS__)
#define Event_dispose(...) spEvent_dispose(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
