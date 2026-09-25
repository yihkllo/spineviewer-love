
#ifndef SPINE_BONE_H_
#define SPINE_BONE_H_

#include <spine/BoneData.h>

#ifdef __cplusplus
extern "C" {
#endif

struct spSkeleton;

typedef struct spBone spBone;
struct spBone {
	spBoneData* const data;
	struct spSkeleton* const skeleton;
	spBone* const parent;
	int childrenCount;
	spBone** const children;
	float x, y, rotation, scaleX, scaleY, shearX, shearY;
	float appliedRotation;

	float const a, b, worldX;
	float const c, d, worldY;
	float const worldSignX, worldSignY;

	int  sorted;

#ifdef __cplusplus
	spBone() :
		data(0),
		skeleton(0),
		parent(0),
		childrenCount(0), children(0),
		x(0), y(0), rotation(0), scaleX(0), scaleY(0),
		appliedRotation(0),

		a(0), b(0), worldX(0),
		c(0), d(0), worldY(0),
		worldSignX(0), worldSignY(0),
		
		sorted(0) {
	}
#endif
};

void spBone_setYDown (int yDown);
int spBone_isYDown ();

spBone* spBone_create (spBoneData* data, struct spSkeleton* skeleton, spBone* parent);
void spBone_dispose (spBone* self);

void spBone_setToSetupPose (spBone* self);

void spBone_updateWorldTransform (spBone* self);
void spBone_updateWorldTransformWith (spBone* self, float x, float y, float rotation, float scaleX, float scaleY, float shearX, float shearY);

float spBone_getWorldRotationX (spBone* self);
float spBone_getWorldRotationY (spBone* self);
float spBone_getWorldScaleX (spBone* self);
float spBone_getWorldScaleY (spBone* self);

float spBone_worldToLocalRotationX (spBone* self);
float spBone_worldToLocalRotationY (spBone* self);
void spBone_rotateWorld (spBone* self, float degrees);
void spBone_updateLocalTransform (spBone* self);

void spBone_worldToLocal (spBone* self, float worldX, float worldY, float* localX, float* localY);
void spBone_localToWorld (spBone* self, float localX, float localY, float* worldX, float* worldY);

#ifdef SPINE_SHORT_NAMES
typedef spBone Bone;
#define Bone_setYDown(...) spBone_setYDown(__VA_ARGS__)
#define Bone_isYDown() spBone_isYDown()
#define Bone_create(...) spBone_create(__VA_ARGS__)
#define Bone_dispose(...) spBone_dispose(__VA_ARGS__)
#define Bone_setToSetupPose(...) spBone_setToSetupPose(__VA_ARGS__)
#define Bone_updateWorldTransform(...) spBone_updateWorldTransform(__VA_ARGS__)
#define Bone_updateWorldTransformWith(...) spBone_updateWorldTransformWith(__VA_ARGS__)
#define Bone_getWorldRotationX(...) spBone_getWorldRotationX(__VA_ARGS__)
#define Bone_getWorldRotationY(...) spBone_getWorldRotationY(__VA_ARGS__)
#define Bone_getWorldScaleX(...) spBone_getWorldScaleX(__VA_ARGS__)
#define Bone_getWorldScaleY(...) spBone_getWorldScaleY(__VA_ARGS__)
#define Bone_worldToLocalRotationX(...) spBone_worldToLocalRotationX(__VA_ARGS__)
#define Bone_worldToLocalRotationY(...) spBone_worldToLocalRotationY(__VA_ARGS__)
#define Bone_rotateWorld(...) spBone_rotateWorld(__VA_ARGS__)
#define Bone_updateLocalTransform(...) spBone_updateLocalTransform(__VA_ARGS__)
#define Bone_worldToLocal(...) spBone_worldToLocal(__VA_ARGS__)
#define Bone_localToWorld(...) spBone_localToWorld(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
