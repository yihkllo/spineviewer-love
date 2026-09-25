
#ifndef SPINE_IKCONSTRAINTDATA_H_
#define SPINE_IKCONSTRAINTDATA_H_

#include <spine/BoneData.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spIkConstraintData {
	const char* const name;
	int order;
	int bonesCount;
	spBoneData** bones;

	spBoneData* target;
	int bendDirection;
	float mix;

#ifdef __cplusplus
	spIkConstraintData() :
		name(0),
		bonesCount(0),
		bones(0),
		target(0),
		bendDirection(0),
		mix(0) {
	}
#endif
} spIkConstraintData;

spIkConstraintData* spIkConstraintData_create (const char* name);
void spIkConstraintData_dispose (spIkConstraintData* self);

#ifdef SPINE_SHORT_NAMES
typedef spIkConstraintData IkConstraintData;
#define IkConstraintData_create(...) spIkConstraintData_create(__VA_ARGS__)
#define IkConstraintData_dispose(...) spIkConstraintData_dispose(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
