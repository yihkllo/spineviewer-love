
#ifndef SPINE_TRANSFORMCONSTRAINTDATA_H_
#define SPINE_TRANSFORMCONSTRAINTDATA_H_

#include <spine/BoneData.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spTransformConstraintData {
	const char* const name;

	spBoneData* bone;
	spBoneData* target;
	float translateMix;
	float x, y;

#ifdef __cplusplus
	spTransformConstraintData() :
		name(0),
		bone(0),
		target(0),
		translateMix(0),
		x(0),
		y(0) {
	}
#endif
} spTransformConstraintData;

spTransformConstraintData* spTransformConstraintData_create (const char* name);
void spTransformConstraintData_dispose (spTransformConstraintData* self);

#ifdef SPINE_SHORT_NAMES
typedef spTransformConstraintData TransformConstraintData;
#define TransformConstraintData_create(...) spTransformConstraintData_create(__VA_ARGS__)
#define TransformConstraintData_dispose(...) spTransformConstraintData_dispose(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
