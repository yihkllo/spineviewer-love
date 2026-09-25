




#ifndef SPINE_JSON_H_
#define SPINE_JSON_H_

#ifdef __cplusplus
extern "C" {
#endif

#define Json_False 0
#define Json_True 1
#define Json_NULL 2
#define Json_Number 3
#define Json_String 4
#define Json_Array 5
#define Json_Object 6

#ifndef SPINE_JSON_HAVE_PREV
#define SPINE_JSON_HAVE_PREV 0
#endif

typedef struct Json {
	struct Json* next;
#if SPINE_JSON_HAVE_PREV
	struct Json* prev;
#endif
	struct Json* child;

	int type;
	int size;

	const char* valueString;
	int valueInt;
	float valueFloat;

	const char* name;
} Json;

Json* Json_create (const char* value);

void Json_dispose (Json* json);

Json* Json_getItem (Json* json, const char* string);
const char* Json_getString (Json* json, const char* name, const char* defaultValue);
float Json_getFloat (Json* json, const char* name, float defaultValue);
int Json_getInt (Json* json, const char* name, int defaultValue);

const char* Json_getError (void);

#ifdef __cplusplus
}
#endif

#endif
