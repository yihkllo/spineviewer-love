
#ifndef SPINE_EVENTDATA_H_
#define SPINE_EVENTDATA_H_

#include <spine/dll.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spEventData {
	const char* const name;
	int intValue;
	float floatValue;
	const char* stringValue;
	const char* audioPath;
	float volume;
	float balance;

#ifdef __cplusplus
	spEventData() :
		name(0),
		intValue(0),
		floatValue(0),
		stringValue(0),
		audioPath(0),
		volume(0),
		balance(0) {
	}
#endif
} spEventData;

SP_API spEventData* spEventData_create (const char* name);
SP_API void spEventData_dispose (spEventData* self);

#ifdef SPINE_SHORT_NAMES
typedef spEventData EventData;
#define EventData_create(...) spEventData_create(__VA_ARGS__)
#define EventData_dispose(...) spEventData_dispose(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
