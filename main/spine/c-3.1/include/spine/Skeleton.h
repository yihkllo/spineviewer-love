
#ifndef SPINE_SKELETON_H_
#define SPINE_SKELETON_H_

#include <spine/SkeletonData.h>
#include <spine/Slot.h>
#include <spine/Skin.h>
#include <spine/IkConstraint.h>
#include <spine/TransformConstraint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spSkeleton {
	spSkeletonData* const data;

	int bonesCount;
	spBone** bones;
	spBone* const root;

	int slotsCount;
	spSlot** slots;
	spSlot** drawOrder;

	int ikConstraintsCount;
	spIkConstraint** ikConstraints;

	int transformConstraintsCount;
	spTransformConstraint** transformConstraints;

	spSkin* const skin;
	float r, g, b, a;
	float time;
	int flipX, flipY;
	float x, y;

#ifdef __cplusplus
	spSkeleton() :
		data(0),
		bonesCount(0),
		bones(0),
		root(0),
		slotsCount(0),
		slots(0),
		drawOrder(0),

		ikConstraintsCount(0),
		ikConstraints(0),

		transformConstraintsCount(0),
		transformConstraints(0),

		skin(0),
		r(0), g(0), b(0), a(0),
		time(0),
		flipX(0),
		flipY(0),
		x(0), y(0) {
	}
#endif
} spSkeleton;

spSkeleton* spSkeleton_create (spSkeletonData* data);
void spSkeleton_dispose (spSkeleton* self);

void spSkeleton_updateCache (const spSkeleton* self);
void spSkeleton_updateWorldTransform (const spSkeleton* self);

void spSkeleton_setToSetupPose (const spSkeleton* self);
void spSkeleton_setBonesToSetupPose (const spSkeleton* self);
void spSkeleton_setSlotsToSetupPose (const spSkeleton* self);

spBone* spSkeleton_findBone (const spSkeleton* self, const char* boneName);
int spSkeleton_findBoneIndex (const spSkeleton* self, const char* boneName);

spSlot* spSkeleton_findSlot (const spSkeleton* self, const char* slotName);
int spSkeleton_findSlotIndex (const spSkeleton* self, const char* slotName);

void spSkeleton_setSkin (spSkeleton* self, spSkin* skin);
int spSkeleton_setSkinByName (spSkeleton* self, const char* skinName);

spAttachment* spSkeleton_getAttachmentForSlotName (const spSkeleton* self, const char* slotName, const char* attachmentName);
spAttachment* spSkeleton_getAttachmentForSlotIndex (const spSkeleton* self, int slotIndex, const char* attachmentName);
int spSkeleton_setAttachment (spSkeleton* self, const char* slotName, const char* attachmentName);

spIkConstraint* spSkeleton_findIkConstraint (const spSkeleton* self, const char* constraintName);

spTransformConstraint* spSkeleton_findTransformConstraint (const spSkeleton* self, const char* constraintName);

void spSkeleton_update (spSkeleton* self, float deltaTime);

#ifdef SPINE_SHORT_NAMES
typedef spSkeleton Skeleton;
#define Skeleton_create(...) spSkeleton_create(__VA_ARGS__)
#define Skeleton_dispose(...) spSkeleton_dispose(__VA_ARGS__)
#define Skeleton_updateWorldTransform(...) spSkeleton_updateWorldTransform(__VA_ARGS__)
#define Skeleton_setToSetupPose(...) spSkeleton_setToSetupPose(__VA_ARGS__)
#define Skeleton_setBonesToSetupPose(...) spSkeleton_setBonesToSetupPose(__VA_ARGS__)
#define Skeleton_setSlotsToSetupPose(...) spSkeleton_setSlotsToSetupPose(__VA_ARGS__)
#define Skeleton_findBone(...) spSkeleton_findBone(__VA_ARGS__)
#define Skeleton_findBoneIndex(...) spSkeleton_findBoneIndex(__VA_ARGS__)
#define Skeleton_findSlot(...) spSkeleton_findSlot(__VA_ARGS__)
#define Skeleton_findSlotIndex(...) spSkeleton_findSlotIndex(__VA_ARGS__)
#define Skeleton_setSkin(...) spSkeleton_setSkin(__VA_ARGS__)
#define Skeleton_setSkinByName(...) spSkeleton_setSkinByName(__VA_ARGS__)
#define Skeleton_getAttachmentForSlotName(...) spSkeleton_getAttachmentForSlotName(__VA_ARGS__)
#define Skeleton_getAttachmentForSlotIndex(...) spSkeleton_getAttachmentForSlotIndex(__VA_ARGS__)
#define Skeleton_setAttachment(...) spSkeleton_setAttachment(__VA_ARGS__)
#define Skeleton_update(...) spSkeleton_update(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
