
#ifndef SPINE_TRANSFORMCONSTRAINT_H_
#define SPINE_TRANSFORMCONSTRAINT_H_

#include <spine/dll.h>
#include <spine/TransformConstraintData.h>
#include <spine/Bone.h>

#ifdef __cplusplus
extern "C" {
#endif

struct spSkeleton;

typedef struct spTransformConstraint {
	spTransformConstraintData* const data;
	int bonesCount;
	spBone** const bones;
	spBone* target;
	float rotateMix, translateMix, scaleMix, shearMix;

#ifdef __cplusplus
	spTransformConstraint() :
		data(0),
		bonesCount(0),
		bones(0),
		target(0),
		rotateMix(0),
		translateMix(0),
		scaleMix(0),
		shearMix(0) {
	}
#endif
} spTransformConstraint;

SP_API spTransformConstraint* spTransformConstraint_create (spTransformConstraintData* data, const struct spSkeleton* skeleton);
SP_API void spTransformConstraint_dispose (spTransformConstraint* self);

SP_API void spTransformConstraint_apply (spTransformConstraint* self);

#ifdef SPINE_SHORT_NAMES
typedef spTransformConstraint TransformConstraint;
#define TransformConstraint_create(...) spTransformConstraint_create(__VA_ARGS__)
#define TransformConstraint_dispose(...) spTransformConstraint_dispose(__VA_ARGS__)
#define TransformConstraint_apply(...) spTransformConstraint_apply(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
