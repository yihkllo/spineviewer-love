#include <spine/BoneTimeline.h>

#include <spine/Event.h>
#include <spine/Skeleton.h>
#include <spine/Bone.h>
#include <spine/BoneData.h>
#include <spine/BonePose.h>
#include <spine/Property.h>

using namespace spine;

const int BoneTimeline2::ENTRIES = 3;
const int BoneTimeline2::VALUE1 = 1;
const int BoneTimeline2::VALUE2 = 2;

RTTI_IMPL_NOPARENT(BoneTimeline)

RTTI_IMPL_MULTI(BoneTimeline1, CurveTimeline1, BoneTimeline)

BoneTimeline1::BoneTimeline1(size_t frameCount, size_t bezierCount, int boneIndex, Property property)
	: CurveTimeline1(frameCount, bezierCount), BoneTimeline(boneIndex), _boneIndex(boneIndex) {
	PropertyId ids[] = {((PropertyId) property << 32) | boneIndex};
	setPropertyIds(ids, 1);
	_additive = true;
}

void BoneTimeline1::apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
						  bool appliedPose) {
	SP_UNUSED(lastTime);
	SP_UNUSED(events);

	Bone *bone = skeleton._bones[_boneIndex];
	if (bone->isActive()) {
		_apply(appliedPose ? *bone->_appliedPose : bone->_pose, bone->_data._setupPose, time, alpha, from, add, out);
	}
}

RTTI_IMPL_MULTI(BoneTimeline2, CurveTimeline, BoneTimeline)

BoneTimeline2::BoneTimeline2(size_t frameCount, size_t bezierCount, int boneIndex, Property property1, Property property2)
	: CurveTimeline(frameCount, BoneTimeline2::ENTRIES, bezierCount), BoneTimeline(boneIndex), _boneIndex(boneIndex) {
	PropertyId ids[] = {((PropertyId) property1 << 32) | boneIndex, ((PropertyId) property2 << 32) | boneIndex};
	setPropertyIds(ids, 2);
	_additive = true;
}

void BoneTimeline2::apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
						  bool appliedPose) {
	SP_UNUSED(lastTime);
	SP_UNUSED(events);

	Bone *bone = skeleton._bones[_boneIndex];
	if (bone->isActive()) {
		_apply(appliedPose ? *bone->_appliedPose : bone->_pose, bone->_data._setupPose, time, alpha, from, add, out);
	}
}

void BoneTimeline2::setFrame(size_t frame, float time, float value1, float value2) {
	frame *= ENTRIES;
	_frames[frame] = time;
	_frames[frame + VALUE1] = value1;
	_frames[frame + VALUE2] = value2;
}
