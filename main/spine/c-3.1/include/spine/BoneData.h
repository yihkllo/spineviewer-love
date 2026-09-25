
#ifndef SPINE_BONEDATA_H_
#define SPINE_BONEDATA_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spBoneData spBoneData;
struct spBoneData {
	const char* const name;
	spBoneData* const parent;
	float length;
	float x, y;
	float rotation;
	float scaleX, scaleY;
	int inheritScale, inheritRotation;

#ifdef __cplusplus
	spBoneData() :
		name(0),
		parent(0),
		length(0),
		x(0), y(0),
		rotation(0),
		scaleX(0), scaleY(0),
		inheritScale(0), inheritRotation(0) {
	}
#endif
};

spBoneData* spBoneData_create (const char* name, spBoneData* parent);
void spBoneData_dispose (spBoneData* self);

#ifdef SPINE_SHORT_NAMES
typedef spBoneData BoneData;
#define BoneData_create(...) spBoneData_create(__VA_ARGS__)
#define BoneData_dispose(...) spBoneData_dispose(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
