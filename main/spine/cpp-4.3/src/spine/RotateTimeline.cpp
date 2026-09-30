#include <spine/RotateTimeline.h>

#include <spine/BonePose.h>

using namespace spine;

RTTI_IMPL(RotateTimeline, BoneTimeline1)

RotateTimeline::RotateTimeline(size_t frameCount, size_t bezierCount, int boneIndex)
	: BoneTimeline1(frameCount, bezierCount, boneIndex, Property_Rotate) {
}

void RotateTimeline::_apply(BonePose &pose, BonePose &setup, float time, float alpha, MixFrom from, bool add, bool out) {
	SP_UNUSED(out);
	pose._rotation = getRelativeValue(time, alpha, from, add, pose._rotation, setup._rotation);
}
