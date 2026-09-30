#include <spine/SlotCurveTimeline.h>

#include <spine/Bone.h>
#include <spine/Skeleton.h>
#include <spine/Slot.h>
#include <spine/SlotPose.h>

using namespace spine;

RTTI_IMPL_MULTI(SlotCurveTimeline, CurveTimeline, SlotTimeline)

SlotCurveTimeline::SlotCurveTimeline(size_t frameCount, size_t frameEntries, size_t bezierCount, int slotIndex)
	: CurveTimeline(frameCount, frameEntries, bezierCount), SlotTimeline(), _slotIndex(slotIndex) {
}

SlotCurveTimeline::~SlotCurveTimeline() {
}

int SlotCurveTimeline::getSlotIndex() {
	return _slotIndex;
}

void SlotCurveTimeline::setSlotIndex(int inValue) {
	_slotIndex = inValue;
}

void SlotCurveTimeline::apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
							  bool appliedPose) {
	SP_UNUSED(lastTime);
	SP_UNUSED(events);
	SP_UNUSED(out);

	Slot *slot = skeleton._slots[_slotIndex];
	if (slot->_bone.isActive()) _apply(*slot, appliedPose ? *slot->_appliedPose : slot->_pose, time, alpha, from, add);
}
