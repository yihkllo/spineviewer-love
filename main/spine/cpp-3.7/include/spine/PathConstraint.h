
#ifndef SPINE_PATHCONSTRAINT_H_
#define SPINE_PATHCONSTRAINT_H_

#include <spine/dll.h>
#include <spine/PathConstraintData.h>
#include <spine/Bone.h>
#include <spine/Slot.h>
#include "PathAttachment.h"

#ifdef __cplusplus
extern "C" {
#endif

struct spSkeleton;

typedef struct spPathConstraint {
	spPathConstraintData* const data;
	int bonesCount;
	spBone** const bones;
	spSlot* target;
	float position, spacing, rotateMix, translateMix;

	int spacesCount;
	float* spaces;

	int positionsCount;
	float* positions;

	int worldCount;
	float* world;

	int curvesCount;
	float* curves;

	int lengthsCount;
	float* lengths;

	float segments[10];

#ifdef __cplusplus
	spPathConstraint() :
		data(0),
		bonesCount(0),
		bones(0),
		target(0),
		position(0),
		spacing(0),
		rotateMix(0),
		translateMix(0),
		spacesCount(0),
		spaces(0),
		positionsCount(0),
		positions(0),
		worldCount(0),
		world(0),
		curvesCount(0),
		curves(0),
		lengthsCount(0),
		lengths(0) {
	}
#endif
} spPathConstraint;

#define SP_PATHCONSTRAINT_

SP_API spPathConstraint* spPathConstraint_create (spPathConstraintData* data, const struct spSkeleton* skeleton);
SP_API void spPathConstraint_dispose (spPathConstraint* self);

SP_API void spPathConstraint_apply (spPathConstraint* self);
SP_API float* spPathConstraint_computeWorldPositions(spPathConstraint* self, spPathAttachment* path, int spacesCount, int  tangents, int percentPosition, int percentSpacing);

#ifdef SPINE_SHORT_NAMES
typedef spPathConstraint PathConstraint;
#define PathConstraint_create(...) spPathConstraint_create(__VA_ARGS__)
#define PathConstraint_dispose(...) spPathConstraint_dispose(__VA_ARGS__)
#define PathConstraint_apply(...) spPathConstraint_apply(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
