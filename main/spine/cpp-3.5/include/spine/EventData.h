
#ifndef SPINE_EVENTDATA_H_
#define SPINE_EVENTDATA_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spEventData {
	const char* const name;
	int intValue;
	float floatValue;
	const char* stringValue;

#ifdef __cplusplus
	spEventData() :
		name(0),
		intValue(0),
		floatValue(0),
		stringValue(0) {
	}
#endif
} spEventData;

spEventData* spEventData_create (const char* name);
void spEventData_dispose (spEventData* self);

#ifdef SPINE_SHORT_NAMES
typedef spEventData EventData;
#define EventData_create(...) spEventData_create(__VA_ARGS__)
#define EventData_dispose(...) spEventData_dispose(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
