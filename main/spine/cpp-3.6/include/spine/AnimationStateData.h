
#ifndef SPINE_ANIMATIONSTATEDATA_H_
#define SPINE_ANIMATIONSTATEDATA_H_

#include <spine/dll.h>
#include <spine/Animation.h>
#include <spine/SkeletonData.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spAnimationStateData {
	spSkeletonData* const skeletonData;
	float defaultMix;
	const void* const entries;

#ifdef __cplusplus
	spAnimationStateData() :
		skeletonData(0),
		defaultMix(0),
		entries(0) {
	}
#endif
} spAnimationStateData;

SP_API spAnimationStateData* spAnimationStateData_create (spSkeletonData* skeletonData);
SP_API void spAnimationStateData_dispose (spAnimationStateData* self);

SP_API void spAnimationStateData_setMixByName (spAnimationStateData* self, const char* fromName, const char* toName, float duration);
SP_API void spAnimationStateData_setMix (spAnimationStateData* self, spAnimation* from, spAnimation* to, float duration);
SP_API float spAnimationStateData_getMix (spAnimationStateData* self, spAnimation* from, spAnimation* to);

#ifdef SPINE_SHORT_NAMES
typedef spAnimationStateData AnimationStateData;
#define AnimationStateData_create(...) spAnimationStateData_create(__VA_ARGS__)
#define AnimationStateData_dispose(...) spAnimationStateData_dispose(__VA_ARGS__)
#define AnimationStateData_setMixByName(...) spAnimationStateData_setMixByName(__VA_ARGS__)
#define AnimationStateData_setMix(...) spAnimationStateData_setMix(__VA_ARGS__)
#define AnimationStateData_getMix(...) spAnimationStateData_getMix(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
