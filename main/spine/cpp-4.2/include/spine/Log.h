
#ifndef SPINE_DEBUG_LOG_H
#define SPINE_DEBUG_LOG_H

#include <spine/spine.h>

namespace spine {
	SP_API void spDebug_printSkeletonData(SkeletonData *skeletonData);

	SP_API void spDebug_printAnimation(Animation *animation);

	SP_API void spDebug_printTimeline(Timeline *timeline);

	SP_API void spDebug_printBoneDatas(Vector<BoneData *> &boneDatas);

	SP_API void spDebug_printBoneData(BoneData *boneData);

	SP_API void spDebug_printSkeleton(Skeleton *skeleton);

	SP_API void spDebug_printBones(Vector<Bone *> &bones);

	SP_API void spDebug_printBone(Bone *bone);

	SP_API void spDebug_printFloats(float *values, int numFloats);

	SP_API void spDebug_printFloats(Vector<float> &values);
}

#endif
