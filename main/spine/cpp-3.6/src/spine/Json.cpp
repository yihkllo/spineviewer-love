




#ifndef _DEFAULT_SOURCE
#define _DEFAULT_SOURCE
#endif

#ifndef _BSD_SOURCE
#define _BSD_SOURCE
#endif

#include "Json.h"
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <spine/extension.h>

#ifndef SPINE_JSON_DEBUG
#define SPINE_JSON_DEBUG 0
#endif

static const char* ep;

const char* Json_getError (void) {
	return ep;
}

static int Json_strcasecmp (const char* s1, const char* s2) {
	if (s1 && s2) {
#if defined(_WIN32)
		return _stricmp(s1, s2);
#else
		return strcasecmp( s1, s2 );
#endif
	} else {
		if (s1 < s2)
			return -1;
		else if (s1 == s2)
			return 0;
		else
			return 1;
	}
}

static Json *Json_new (void) {
	return (Json*)CALLOC(Json, 1);
}

void Json_dispose (Json *c) {
	Json *next;
	while (c) {
		next = c->next;
		if (c->child) Json_dispose(c->child);
		if (c->valueString) FREE(c->valueString);
		if (c->name) FREE(c->name);
		FREE(c);
		c = next;
	}
}

static const char* parse_number (Json *item, const char* num) {
	double result = 0.0;
	int negative = 0;
	char* ptr = (char*)num;

	if (*ptr == '-') {
		negative = -1;
		++ptr;
	}

	while (*ptr >= '0' && *ptr <= '9') {
		result = result * 10.0 + (*ptr - '0');
		++ptr;
	}

	if (*ptr == '.') {
		double fraction = 0.0;
		int n = 0;
		++ptr;

		while (*ptr >= '0' && *ptr <= '9') {
			fraction = (fraction * 10.0) + (*ptr - '0');
			++ptr;
			++n;
		}
		result += fraction / POW(10.0, n);
	}
	if (negative) result = -result;

	if (*ptr == 'e' || *ptr == 'E') {
		double exponent = 0;
		int expNegative = 0;
		int n = 0;
		++ptr;

		if (*ptr == '-') {
			expNegative = -1;
			++ptr;
		} else if (*ptr == '+') {
			++ptr;
		}

		while (*ptr >= '0' && *ptr <= '9') {
			exponent = (exponent * 10.0) + (*ptr - '0');
			++ptr;
			++n;
		}

		if (expNegative)
			result = result / POW(10, exponent);
		else
			result = result * POW(10, exponent);
	}

	if (ptr != num) {
		item->valueFloat = result;
		item->valueInt = (int)result;
		item->type = Json_Number;
		return ptr;
	} else {
		ep = num;
		return 0;
	}
}

static const unsigned char firstByteMark[7] = {0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};
static const char* parse_string (Json *item, const char* str) {
	const char* ptr = str + 1;
	char* ptr2;
	char* out;
	int len = 0;
	unsigned uc, uc2;
	if (*str != '\"') {
		ep = str;
		return 0;
	}

	while (*ptr != '\"' && *ptr && ++len)
		if (*ptr++ == '\\') ptr++;

	out = MALLOC(char, len + 1);
	if (!out) return 0;

	ptr = str + 1;
	ptr2 = out;
	while (*ptr != '\"' && *ptr) {
		if (*ptr != '\\')
			*ptr2++ = *ptr++;
		else {
			ptr++;
			switch (*ptr) {
			case 'b':
				*ptr2++ = '\b';
				break;
			case 'f':
				*ptr2++ = '\f';
				break;
			case 'n':
				*ptr2++ = '\n';
				break;
			case 'r':
				*ptr2++ = '\r';
				break;
			case 't':
				*ptr2++ = '\t';
				break;
			case 'u':
				sscanf(ptr + 1, "%4x", &uc);
				ptr += 4;

				if ((uc >= 0xDC00 && uc <= 0xDFFF) || uc == 0) break;

				if (uc >= 0xD800 && uc <= 0xDBFF)
				{
					if (ptr[1] != '\\' || ptr[2] != 'u') break;
					sscanf(ptr + 3, "%4x", &uc2);
					ptr += 6;
					if (uc2 < 0xDC00 || uc2 > 0xDFFF) break;
					uc = 0x10000 + (((uc & 0x3FF) << 10) | (uc2 & 0x3FF));
				}

				len = 4;
				if (uc < 0x80)
					len = 1;
				else if (uc < 0x800)
					len = 2;
				else if (uc < 0x10000) len = 3;
				ptr2 += len;

				switch (len) {
				case 4:
					*--ptr2 = ((uc | 0x80) & 0xBF);
					uc >>= 6;
				case 3:
					*--ptr2 = ((uc | 0x80) & 0xBF);
					uc >>= 6;
				case 2:
					*--ptr2 = ((uc | 0x80) & 0xBF);
					uc >>= 6;
				case 1:
					*--ptr2 = (uc | firstByteMark[len]);
				}
				ptr2 += len;
				break;
			default:
				*ptr2++ = *ptr;
				break;
			}
			ptr++;
		}
	}
	*ptr2 = 0;
	if (*ptr == '\"') ptr++;
	item->valueString = out;
	item->type = Json_String;
	return ptr;
}

static const char* parse_value (Json *item, const char* value);
static const char* parse_array (Json *item, const char* value);
static const char* parse_object (Json *item, const char* value);

static const char* skip (const char* in) {
	if (!in) return 0;
	while (*in && (unsigned char)*in <= 32)
		in++;
	return in;
}

Json *Json_create (const char* value) {
	Json *c;
	ep = 0;
	if (!value) return 0;
	c = Json_new();
	if (!c) return 0;

	value = parse_value(c, skip(value));
	if (!value) {
		Json_dispose(c);
		return 0;
	}

	return c;
}

static const char* parse_value (Json *item, const char* value) {
#if SPINE_JSON_DEBUG
	if (!value) return 0;
#endif

	switch (*value) {
	case 'n': {
		if (!strncmp(value + 1, "ull", 3)) {
			item->type = Json_NULL;
			return value + 4;
		}
		break;
	}
	case 'f': {
		if (!strncmp(value + 1, "alse", 4)) {
			item->type = Json_False;
			return value + 5;
		}
		break;
	}
	case 't': {
		if (!strncmp(value + 1, "rue", 3)) {
			item->type = Json_True;
			item->valueInt = 1;
			return value + 4;
		}
		break;
	}
	case '\"':
		return parse_string(item, value);
	case '[':
		return parse_array(item, value);
	case '{':
		return parse_object(item, value);
	case '-':
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		return parse_number(item, value);
	default:
		break;
	}

	ep = value;
	return 0;
}

static const char* parse_array (Json *item, const char* value) {
	Json *child;

#if SPINE_JSON_DEBUG
	if (*value != '[') {
		ep = value;
		return 0;
	}
#endif

	item->type = Json_Array;
	value = skip(value + 1);
	if (*value == ']') return value + 1;

	item->child = child = Json_new();
	if (!item->child) return 0;
	value = skip(parse_value(child, skip(value)));
	if (!value) return 0;
	item->size = 1;

	while (*value == ',') {
		Json *new_item = Json_new();
		if (!new_item) return 0;
		child->next = new_item;
#if SPINE_JSON_HAVE_PREV
		new_item->prev = child;
#endif
		child = new_item;
		value = skip(parse_value(child, skip(value + 1)));
		if (!value) return 0;
		item->size++;
	}

	if (*value == ']') return value + 1;
	ep = value;
	return 0;
}

static const char* parse_object (Json *item, const char* value) {
	Json *child;

#if SPINE_JSON_DEBUG
	if (*value != '{') {
		ep = value;
		return 0;
	}
#endif

	item->type = Json_Object;
	value = skip(value + 1);
	if (*value == '}') return value + 1;

	item->child = child = Json_new();
	if (!item->child) return 0;
	value = skip(parse_string(child, skip(value)));
	if (!value) return 0;
	child->name = child->valueString;
	child->valueString = 0;
	if (*value != ':') {
		ep = value;
		return 0;
	}
	value = skip(parse_value(child, skip(value + 1)));
	if (!value) return 0;
	item->size = 1;

	while (*value == ',') {
		Json *new_item = Json_new();
		if (!new_item) return 0;
		child->next = new_item;
#if SPINE_JSON_HAVE_PREV
		new_item->prev = child;
#endif
		child = new_item;
		value = skip(parse_string(child, skip(value + 1)));
		if (!value) return 0;
		child->name = child->valueString;
		child->valueString = 0;
		if (*value != ':') {
			ep = value;
			return 0;
		}
		value = skip(parse_value(child, skip(value + 1)));
		if (!value) return 0;
		item->size++;
	}

	if (*value == '}') return value + 1;
	ep = value;
	return 0;
}

Json *Json_getItem (Json *object, const char* string) {
	Json *c = object->child;
	while (c && Json_strcasecmp(c->name, string))
		c = c->next;
	return c;
}

const char* Json_getString (Json* object, const char* name, const char* defaultValue) {
	object = Json_getItem(object, name);
	if (object) return object->valueString;
	return defaultValue;
}

float Json_getFloat (Json* value, const char* name, float defaultValue) {
	value = Json_getItem(value, name);
	return value ? value->valueFloat : defaultValue;
}

int Json_getInt (Json* value, const char* name, int defaultValue) {
	value = Json_getItem(value, name);
	return value ? value->valueInt : defaultValue;
}
