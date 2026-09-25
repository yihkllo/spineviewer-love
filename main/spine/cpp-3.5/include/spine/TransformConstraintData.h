
#ifndef SPINE_TRANSFORMCONSTRAINTDATA_H_
#define SPINE_TRANSFORMCONSTRAINTDATA_H_

#include <spine/BoneData.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spTransformConstraintData {
	const char* const name;
	int order;
	int bonesCount;
	spBoneData** const bones;
	spBoneData* target;
	float rotateMix, translateMix, scaleMix, shearMix;
	float offsetRotation, offsetX, offsetY, offsetScaleX, offsetScaleY, offsetShearY;
	int   relative;
	int   local;

#ifdef __cplusplus
	spTransformConstraintData() :
		name(0),
		bonesCount(0),
		bones(0),
		target(0),
		rotateMix(0),
		translateMix(0),
		scaleMix(0),
		shearMix(0),
		offsetRotation(0),
		offsetX(0),
		offsetY(0),
		offsetScaleX(0),
		offsetScaleY(0),
		offsetShearY(0),
		relative(0),
		local(0) {
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
