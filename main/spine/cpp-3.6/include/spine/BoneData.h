
#ifndef SPINE_BONEDATA_H_
#define SPINE_BONEDATA_H_

#include <spine/dll.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	SP_TRANSFORMMODE_NORMAL,
	SP_TRANSFORMMODE_ONLYTRANSLATION,
	SP_TRANSFORMMODE_NOROTATIONORREFLECTION,
	SP_TRANSFORMMODE_NOSCALE,
	SP_TRANSFORMMODE_NOSCALEORREFLECTION
} spTransformMode;

typedef struct spBoneData spBoneData;
struct spBoneData {
	const int index;
	const char* const name;
	spBoneData* const parent;
	float length;
	float x, y, rotation, scaleX, scaleY, shearX, shearY;
	spTransformMode transformMode;

#ifdef __cplusplus
	spBoneData() :
		index(0),
		name(0),
		parent(0),
		length(0),
		x(0), y(0),
		rotation(0),
		scaleX(0), scaleY(0),
		shearX(0), shearY(0),
		transformMode(SP_TRANSFORMMODE_NORMAL) {
	}
#endif
};

SP_API spBoneData* spBoneData_create (int index, const char* name, spBoneData* parent);
SP_API void spBoneData_dispose (spBoneData* self);

#ifdef SPINE_SHORT_NAMES
typedef spBoneData BoneData;
#define BoneData_create(...) spBoneData_create(__VA_ARGS__)
#define BoneData_dispose(...) spBoneData_dispose(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
