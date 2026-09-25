
#ifndef SPINE_PATHCONSTRAINTDATA_H_
#define SPINE_PATHCONSTRAINTDATA_H_

#include <spine/dll.h>
#include <spine/BoneData.h>
#include <spine/SlotData.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	SP_POSITION_MODE_FIXED, SP_POSITION_MODE_PERCENT
} spPositionMode;

typedef enum {
	SP_SPACING_MODE_LENGTH, SP_SPACING_MODE_FIXED, SP_SPACING_MODE_PERCENT
} spSpacingMode;

typedef enum {
	SP_ROTATE_MODE_TANGENT, SP_ROTATE_MODE_CHAIN, SP_ROTATE_MODE_CHAIN_SCALE
} spRotateMode;

typedef struct spPathConstraintData {
	const char* const name;
	int order;
	int bonesCount;
	spBoneData** const bones;
	spSlotData* target;
	spPositionMode positionMode;
	spSpacingMode spacingMode;
	spRotateMode rotateMode;
	float offsetRotation;
	float position, spacing, rotateMix, translateMix;

#ifdef __cplusplus
	spPathConstraintData() :
		name(0),
		bonesCount(0),
		bones(0),
		target(0),
		positionMode(SP_POSITION_MODE_FIXED),
		spacingMode(SP_SPACING_MODE_LENGTH),
		rotateMode(SP_ROTATE_MODE_TANGENT),
		offsetRotation(0),
		position(0),
		spacing(0),
		rotateMix(0),
		translateMix(0) {
	}
#endif
} spPathConstraintData;

SP_API spPathConstraintData* spPathConstraintData_create (const char* name);
SP_API void spPathConstraintData_dispose (spPathConstraintData* self);

#ifdef SPINE_SHORT_NAMES
typedef spPathConstraintData PathConstraintData;
#define PathConstraintData_create(...) spPathConstraintData_create(__VA_ARGS__)
#define PathConstraintData_dispose(...) spPathConstraintData_dispose(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
