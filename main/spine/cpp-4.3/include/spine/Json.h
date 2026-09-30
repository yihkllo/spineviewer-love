#ifndef Spine_Json_h
#define Spine_Json_h

#include <spine/SpineObject.h>
#include <spine/Array.h>

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

		static bool asFloatArray(Json *value, Array<float> &array) {
			if (value == NULL) return false;
			array.setSize(value->_size, 0);
			Json *vertex = value->_child;
			for (int i = 0; vertex; vertex = vertex->_next, i++) array[i] = vertex->_valueFloat;
			return true;
		}

		static bool asIntArray(Json *value, Array<int> &array) {
			if (value == NULL) return false;
			array.setSize(value->_size, 0);
			Json *vertex = value->_child;
			for (int i = 0; vertex; vertex = vertex->_next, i++) array[i] = vertex->_valueInt;
			return true;
		}

		static bool asUnsignedShortArray(Json *value, Array<unsigned short> &array) {
			if (value == NULL) return false;
			array.setSize(value->_size, 0);
			Json *vertex = value->_child;
			for (int i = 0; vertex; vertex = vertex->_next, i++) array[i] = (unsigned short) vertex->_valueInt;
			return true;
		}

		static Json *getItem(Json *object, const char *string);

		static Json *getItem(Json *object, int childIndex);

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
		Json *_prev;
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
