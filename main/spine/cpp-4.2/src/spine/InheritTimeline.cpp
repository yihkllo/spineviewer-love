
#include <spine/InheritTimeline.h>

#include <spine/Event.h>
#include <spine/Skeleton.h>

#include <spine/Bone.h>
#include <spine/BoneData.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>

using namespace spine;

RTTI_IMPL(InheritTimeline, Timeline)

InheritTimeline::InheritTimeline(size_t frameCount, int boneIndex) : Timeline(frameCount, ENTRIES),
																	 _boneIndex(boneIndex) {
	PropertyId ids[] = {((PropertyId) Property_Inherit << 32) | boneIndex};
	setPropertyIds(ids, 1);
}

InheritTimeline::~InheritTimeline() {
}

void InheritTimeline::setFrame(int frame, float time, Inherit inherit) {
	frame *= ENTRIES;
	_frames[frame] = time;
	_frames[frame + INHERIT] = inherit;
}


void InheritTimeline::apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha,
							MixBlend blend, MixDirection direction) {
	SP_UNUSED(lastTime);
	SP_UNUSED(pEvents);
	SP_UNUSED(direction);
	SP_UNUSED(alpha);

	Bone *bone = skeleton.getBones()[_boneIndex];
	if (!bone->isActive()) return;

	if (direction == MixDirection_Out) {
		if (blend == MixBlend_Setup) bone->setInherit(bone->_data.getInherit());
		return;
	}

	if (time < _frames[0]) {
		if (blend == MixBlend_Setup || blend == MixBlend_First) bone->_inherit = bone->_data.getInherit();
		return;
	}
	int idx = Animation::search(_frames, time, ENTRIES) + INHERIT;
	bone->_inherit = static_cast<Inherit>(_frames[idx]);
}
