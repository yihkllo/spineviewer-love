
#ifndef SPINE_BONE_H_
#define SPINE_BONE_H_

#include <spine/dll.h>
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
	float ax, ay, arotation, ascaleX, ascaleY, ashearX, ashearY;
	int   appliedValid;

	float const a, b, worldX;
	float const c, d, worldY;

	int  sorted;

#ifdef __cplusplus
	spBone() :
		data(0),
		skeleton(0),
		parent(0),
		childrenCount(0), children(0),
		x(0), y(0), rotation(0), scaleX(0), scaleY(0),
		ax(0), ay(0), arotation(0), ascaleX(0), ascaleY(0), ashearX(0), ashearY(0),
		appliedValid(0),

		a(0), b(0), worldX(0),
		c(0), d(0), worldY(0),

		sorted(0) {
	}
#endif
};

SP_API void spBone_setYDown (int yDown);
SP_API int spBone_isYDown ();

SP_API spBone* spBone_create (spBoneData* data, struct spSkeleton* skeleton, spBone* parent);
SP_API void spBone_dispose (spBone* self);

SP_API void spBone_setToSetupPose (spBone* self);

SP_API void spBone_updateWorldTransform (spBone* self);
SP_API void spBone_updateWorldTransformWith (spBone* self, float x, float y, float rotation, float scaleX, float scaleY, float shearX, float shearY);

SP_API float spBone_getWorldRotationX (spBone* self);
SP_API float spBone_getWorldRotationY (spBone* self);
SP_API float spBone_getWorldScaleX (spBone* self);
SP_API float spBone_getWorldScaleY (spBone* self);

SP_API void spBone_updateAppliedTransform (spBone* self);

SP_API void spBone_worldToLocal (spBone* self, float worldX, float worldY, float* localX, float* localY);
SP_API void spBone_localToWorld (spBone* self, float localX, float localY, float* worldX, float* worldY);
SP_API float spBone_worldToLocalRotation (spBone* self, float worldRotation);
SP_API float spBone_localToWorldRotation (spBone* self, float localRotation);
SP_API void spBone_rotateWorld (spBone* self, float degrees);

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
#define Bone_updateAppliedTransform(...) spBone_updateAppliedTransform(__VA_ARGS__)
#define Bone_worldToLocal(...) spBone_worldToLocal(__VA_ARGS__)
#define Bone_localToWorld(...) spBone_localToWorld(__VA_ARGS__)
#define Bone_worldToLocalRotation(...) spBone_worldToLocalRotation(__VA_ARGS__)
#define Bone_localToWorldRotation(...) spBone_localToWorldRotation(__VA_ARGS__)
#define Bone_rotateWorld(...) spBone_rotateWorld(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
