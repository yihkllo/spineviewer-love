
#ifndef SPINE_TRANSFORMCONSTRAINT_H_
#define SPINE_TRANSFORMCONSTRAINT_H_

#include <spine/TransformConstraintData.h>
#include <spine/Bone.h>

#ifdef __cplusplus
extern "C" {
#endif

struct spSkeleton;

typedef struct spTransformConstraint {
	spTransformConstraintData* const data;
	spBone* bone;
	spBone* target;
	float translateMix;
	float x, y;

#ifdef __cplusplus
	spTransformConstraint() :
		data(0),
		bone(0),
		target(0),
		translateMix(0),
		x(0),
		y(0) {
	}
#endif
} spTransformConstraint;

spTransformConstraint* spTransformConstraint_create (spTransformConstraintData* data, const struct spSkeleton* skeleton);
void spTransformConstraint_dispose (spTransformConstraint* self);

void spTransformConstraint_apply (spTransformConstraint* self);

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
