
#ifndef SPINE_SKELETONDATA_H_
#define SPINE_SKELETONDATA_H_

#include <spine/BoneData.h>
#include <spine/SlotData.h>
#include <spine/Skin.h>
#include <spine/EventData.h>
#include <spine/Animation.h>
#include <spine/IkConstraintData.h>
#include <spine/TransformConstraintData.h>
#include <spine/PathConstraintData.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spSkeletonData {
	const char* version;
	const char* hash;
	float width, height;

	int bonesCount;
	spBoneData** bones;

	int slotsCount;
	spSlotData** slots;

	int skinsCount;
	spSkin** skins;
	spSkin* defaultSkin;

	int eventsCount;
	spEventData** events;

	int animationsCount;
	spAnimation** animations;

	int ikConstraintsCount;
	spIkConstraintData** ikConstraints;

	int transformConstraintsCount;
	spTransformConstraintData** transformConstraints;

	int pathConstraintsCount;
	spPathConstraintData** pathConstraints;
} spSkeletonData;

spSkeletonData* spSkeletonData_create ();
void spSkeletonData_dispose (spSkeletonData* self);

spBoneData* spSkeletonData_findBone (const spSkeletonData* self, const char* boneName);
int spSkeletonData_findBoneIndex (const spSkeletonData* self, const char* boneName);

spSlotData* spSkeletonData_findSlot (const spSkeletonData* self, const char* slotName);
int spSkeletonData_findSlotIndex (const spSkeletonData* self, const char* slotName);

spSkin* spSkeletonData_findSkin (const spSkeletonData* self, const char* skinName);

spEventData* spSkeletonData_findEvent (const spSkeletonData* self, const char* eventName);

spAnimation* spSkeletonData_findAnimation (const spSkeletonData* self, const char* animationName);

spIkConstraintData* spSkeletonData_findIkConstraint (const spSkeletonData* self, const char* constraintName);

spTransformConstraintData* spSkeletonData_findTransformConstraint (const spSkeletonData* self, const char* constraintName);

spPathConstraintData* spSkeletonData_findPathConstraint (const spSkeletonData* self, const char* constraintName);

#ifdef SPINE_SHORT_NAMES
typedef spSkeletonData SkeletonData;
#define SkeletonData_create(...) spSkeletonData_create(__VA_ARGS__)
#define SkeletonData_dispose(...) spSkeletonData_dispose(__VA_ARGS__)
#define SkeletonData_findBone(...) spSkeletonData_findBone(__VA_ARGS__)
#define SkeletonData_findBoneIndex(...) spSkeletonData_findBoneIndex(__VA_ARGS__)
#define SkeletonData_findSlot(...) spSkeletonData_findSlot(__VA_ARGS__)
#define SkeletonData_findSlotIndex(...) spSkeletonData_findSlotIndex(__VA_ARGS__)
#define SkeletonData_findSkin(...) spSkeletonData_findSkin(__VA_ARGS__)
#define SkeletonData_findEvent(...) spSkeletonData_findEvent(__VA_ARGS__)
#define SkeletonData_findAnimation(...) spSkeletonData_findAnimation(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
