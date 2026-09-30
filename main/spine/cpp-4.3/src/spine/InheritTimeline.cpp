#include <spine/InheritTimeline.h>

#include <spine/Event.h>
#include <spine/Skeleton.h>

#include <spine/Bone.h>
#include <spine/BoneData.h>
#include <spine/BonePose.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>

using namespace spine;

RTTI_IMPL_MULTI(InheritTimeline, Timeline, BoneTimeline)

InheritTimeline::InheritTimeline(size_t frameCount, int boneIndex) : Timeline(frameCount, ENTRIES), BoneTimeline(boneIndex), _boneIndex(boneIndex) {
	PropertyId ids[] = {((PropertyId) Property_Inherit << 32) | boneIndex};
	setPropertyIds(ids, 1);
	_instant = true;
}

InheritTimeline::~InheritTimeline() {
}

void InheritTimeline::setFrame(int frame, float time, Inherit inherit) {
	frame *= ENTRIES;
	_frames[frame] = time;
	_frames[frame + INHERIT] = (float) inherit;
}

void InheritTimeline::apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
							bool appliedPose) {
	SP_UNUSED(lastTime);
	SP_UNUSED(events);
	SP_UNUSED(alpha);
	SP_UNUSED(add);

	Bone *bone = skeleton._bones[_boneIndex];
	if (!bone->isActive()) return;
	BonePose &pose = appliedPose ? *bone->_appliedPose : bone->_pose;

	if (out) {
		if (from != MixFrom_Current) pose._inherit = bone->_data._setupPose._inherit;
	} else {
		if (time < _frames[0]) {
			if (from != MixFrom_Current) pose._inherit = bone->_data._setupPose._inherit;
		} else {
			int idx = Animation::search(_frames, time, ENTRIES) + INHERIT;
			pose._inherit = static_cast<Inherit>((int) _frames[idx]);
		}
	}
}
