#include <spine/ShearTimeline.h>

#include <spine/Event.h>
#include <spine/Skeleton.h>
#include <spine/Animation.h>

#include <spine/Bone.h>
#include <spine/BoneData.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>

using namespace spine;

RTTI_IMPL(ShearTimeline, BoneTimeline2)

ShearTimeline::ShearTimeline(size_t frameCount, size_t bezierCount, int boneIndex)
	: BoneTimeline2(frameCount, bezierCount, boneIndex, Property_ShearX, Property_ShearY) {
}

void ShearTimeline::_apply(BonePose &pose, BonePose &setup, float time, float alpha, MixFrom from, bool add, bool out) {
	SP_UNUSED(out);
	if (time < _frames[0]) {
		switch (from) {
			case MixFrom_Setup:
				pose._shearX = setup._shearX;
				pose._shearY = setup._shearY;
				break;
			case MixFrom_First:
				pose._shearX += (setup._shearX - pose._shearX) * alpha;
				pose._shearY += (setup._shearY - pose._shearY) * alpha;
				break;
			case MixFrom_Current:
				break;
		}
		return;
	}

	float x, y;
	int i = Animation::search(_frames, time, BoneTimeline2::ENTRIES);
	int curveType = (int) _curves[i / BoneTimeline2::ENTRIES];
	switch (curveType) {
		case CurveTimeline::LINEAR: {
			float before = _frames[i];
			x = _frames[i + BoneTimeline2::VALUE1];
			y = _frames[i + BoneTimeline2::VALUE2];
			float t = (time - before) / (_frames[i + BoneTimeline2::ENTRIES] - before);
			x += (_frames[i + BoneTimeline2::ENTRIES + BoneTimeline2::VALUE1] - x) * t;
			y += (_frames[i + BoneTimeline2::ENTRIES + BoneTimeline2::VALUE2] - y) * t;
			break;
		}
		case CurveTimeline::STEPPED: {
			x = _frames[i + BoneTimeline2::VALUE1];
			y = _frames[i + BoneTimeline2::VALUE2];
			break;
		}
		default: {
			x = getBezierValue(time, i, BoneTimeline2::VALUE1, curveType - BoneTimeline2::BEZIER);
			y = getBezierValue(time, i, BoneTimeline2::VALUE2, curveType + BoneTimeline2::BEZIER_SIZE - BoneTimeline2::BEZIER);
		}
	}

	if (from == MixFrom_Setup) {
		pose._shearX = setup._shearX + x * alpha;
		pose._shearY = setup._shearY + y * alpha;
	} else if (add) {
		pose._shearX += x * alpha;
		pose._shearY += y * alpha;
	} else {
		pose._shearX += (setup._shearX + x - pose._shearX) * alpha;
		pose._shearY += (setup._shearY + y - pose._shearY) * alpha;
	}
}

RTTI_IMPL(ShearXTimeline, BoneTimeline1)

ShearXTimeline::ShearXTimeline(size_t frameCount, size_t bezierCount, int boneIndex)
	: BoneTimeline1(frameCount, bezierCount, boneIndex, Property_ShearX) {
}

void ShearXTimeline::_apply(BonePose &pose, BonePose &setup, float time, float alpha, MixFrom from, bool add, bool out) {
	SP_UNUSED(out);
	pose._shearX = getRelativeValue(time, alpha, from, add, pose._shearX, setup._shearX);
}

RTTI_IMPL(ShearYTimeline, BoneTimeline1)

ShearYTimeline::ShearYTimeline(size_t frameCount, size_t bezierCount, int boneIndex)
	: BoneTimeline1(frameCount, bezierCount, boneIndex, Property_ShearY) {
}

void ShearYTimeline::_apply(BonePose &pose, BonePose &setup, float time, float alpha, MixFrom from, bool add, bool out) {
	SP_UNUSED(out);
	pose._shearY = getRelativeValue(time, alpha, from, add, pose._shearY, setup._shearY);
}
