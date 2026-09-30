#include <spine/AttachmentTimeline.h>

#include <spine/Event.h>
#include <spine/Skeleton.h>

#include <spine/Animation.h>
#include <spine/Bone.h>
#include <spine/Property.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>
#include <spine/SlotPose.h>

using namespace spine;

RTTI_IMPL_MULTI(AttachmentTimeline, Timeline, SlotTimeline)

AttachmentTimeline::AttachmentTimeline(size_t frameCount, int slotIndex) : Timeline(frameCount, 1), SlotTimeline(), _slotIndex(slotIndex) {
	PropertyId ids[] = {((PropertyId) Property_Attachment << 32) | slotIndex};
	setPropertyIds(ids, 1);
	_instant = true;

	_attachmentNames.ensureCapacity(frameCount);
	for (size_t i = 0; i < frameCount; ++i) {
		_attachmentNames.add(String());
	}
}

AttachmentTimeline::~AttachmentTimeline() {
}

void AttachmentTimeline::setAttachment(Skeleton &skeleton, SlotPose &pose, String *attachmentName) {
	pose.setAttachment(attachmentName == NULL || attachmentName->isEmpty() ? NULL : skeleton.getAttachment(_slotIndex, *attachmentName));
}

void AttachmentTimeline::apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
							   bool appliedPose) {
	SP_UNUSED(lastTime);
	SP_UNUSED(events);
	SP_UNUSED(alpha);
	SP_UNUSED(add);

	Slot *slot = skeleton._slots[_slotIndex];
	if (!slot->_bone.isActive()) return;
	SlotPose &pose = appliedPose ? *slot->_appliedPose : slot->_pose;

	if (out || time < _frames[0]) {
		if (from != MixFrom_Current) setAttachment(skeleton, pose, &slot->_data._attachmentName);
	} else {
		setAttachment(skeleton, pose, &_attachmentNames[Animation::search(_frames, time)]);
	}
}

void AttachmentTimeline::setFrame(int frame, float time, const String &attachmentName) {
	_frames[frame] = time;
	_attachmentNames[frame] = attachmentName;
}

Array<String> &AttachmentTimeline::getAttachmentNames() {
	return _attachmentNames;
}
