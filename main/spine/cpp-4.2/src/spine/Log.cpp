
#include <spine/Log.h>

#include <stdio.h>

using namespace spine;

void spine::spDebug_printSkeletonData(SkeletonData *skeletonData) {
	int i, n;
	spDebug_printBoneDatas(skeletonData->getBones());

	for (i = 0, n = (int) skeletonData->getAnimations().size(); i < n; i++) {
		spDebug_printAnimation(skeletonData->getAnimations()[i]);
	}
}

void _spDebug_printTimelineBase(Timeline *timeline) {
	printf("   Timeline %s:\n", timeline->getRTTI().getClassName());
	printf("      frame count: %zu\n", timeline->getFrameCount());
	printf("      frame entries: %zu\n", timeline->getFrameEntries());
	printf("      frames: ");
	spDebug_printFloats(timeline->getFrames());
	printf("\n");
}

void _spDebug_printCurveTimeline(CurveTimeline *timeline) {
	_spDebug_printTimelineBase(timeline);
	printf("      curves: ");
	spDebug_printFloats(timeline->getCurves());
	printf("\n");
}

void spine::spDebug_printTimeline(Timeline *timeline) {
	if (timeline->getRTTI().instanceOf(CurveTimeline::rtti))
		_spDebug_printCurveTimeline(static_cast<CurveTimeline *>(timeline));
	else
		_spDebug_printTimelineBase(timeline);
}

void spine::spDebug_printAnimation(Animation *animation) {
	int i, n;
	printf("Animation %s: %zu timelines\n", animation->getName().buffer(), animation->getTimelines().size());

	for (i = 0, n = (int) animation->getTimelines().size(); i < n; i++) {
		Timeline *timeline = animation->getTimelines()[i];
		spDebug_printTimeline(timeline);
	}
}

void spine::spDebug_printBoneDatas(Vector<BoneData *> &boneDatas) {
	int i, n;
	for (i = 0, n = (int) boneDatas.size(); i < n; i++) {
		spDebug_printBoneData(boneDatas[i]);
	}
}

void spine::spDebug_printBoneData(BoneData *boneData) {
	printf("Bone data %s: %f, %f, %f, %f, %f, %f %f\n", boneData->getName().buffer(), boneData->getRotation(),
		   boneData->getScaleX(), boneData->getScaleY(), boneData->getX(), boneData->getY(), boneData->getShearX(),
		   boneData->getShearY());
}

void spine::spDebug_printSkeleton(Skeleton *skeleton) {
	spDebug_printBones(skeleton->getBones());
}

void spine::spDebug_printBones(Vector<Bone *> &bones) {
	int i, n;
	for (i = 0, n = (int) bones.size(); i < n; i++) {
		spDebug_printBone(bones[i]);
	}
}

void spine::spDebug_printBone(Bone *bone) {
	printf("Bone %s: %f, %f, %f, %f, %f, %f\n", bone->getData().getName().buffer(), bone->getA(), bone->getB(),
		   bone->getC(), bone->getD(), bone->getWorldX(), bone->getWorldY());
}

void spine::spDebug_printFloats(float *values, int numFloats) {
	int i;
	printf("(%i) [", numFloats);
	for (i = 0; i < numFloats; i++) {
		printf("%f, ", values[i]);
	}
	printf("]");
}

void spine::spDebug_printFloats(Vector<float> &values) {
	int i, n;
	printf("(%zu) [", values.size());
	for (i = 0, n = (int) values.size(); i < n; i++) {
		printf("%f, ", values[i]);
	}
	printf("]");
}
