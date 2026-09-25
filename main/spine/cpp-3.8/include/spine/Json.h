
#ifndef Spine_Json_h
#define Spine_Json_h

#include <spine/SpineObject.h>

#ifndef SPINE_JSON_HAVE_PREV
#define SPINE_JSON_HAVE_PREV 0
#endif

namespace spine {
class SP_API Json : public SpineObject {
	friend class SkeletonJson;

public:
	static const int JSON_FALSE;
	static const int JSON_TRUE;
	static const int JSON_NULL;
	static const int JSON_NUMBER;
	static const int JSON_STRING;
	static const int JSON_ARRAY;
	static const int JSON_OBJECT;

	static Json *getItem(Json *object, const char *string);

	static const char *getString(Json *object, const char *name, const char *defaultValue);

	static float getFloat(Json *object, const char *name, float defaultValue);

	static int getInt(Json *object, const char *name, int defaultValue);

	static bool getBoolean(Json *object, const char *name, bool defaultValue);

	static const char *getError();

	explicit Json(const char *value);

	~Json();



private:
	static const char *_error;

	Json *_next;
#if SPINE_JSON_HAVE_PREV
	Json* _prev;
#endif
	Json *_child;

	int _type;
	int _size;

	const char *_valueString;
	int _valueInt;
	float _valueFloat;

	const char *_name;

	static const char *skip(const char *inValue);

	static const char *parseValue(Json *item, const char *value);

	static const char *parseString(Json *item, const char *str);

	static const char *parseNumber(Json *item, const char *num);

	static const char *parseArray(Json *item, const char *value);

	static const char *parseObject(Json *item, const char *value);

	static int json_strcasecmp(const char *s1, const char *s2);
};
}

#endif
