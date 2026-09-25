
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/AttachmentTimeline.h>

#include <spine/Event.h>
#include <spine/Skeleton.h>

#include <spine/Animation.h>
#include <spine/Bone.h>
#include <spine/Property.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>

using namespace spine;

RTTI_IMPL(AttachmentTimeline, Timeline)

AttachmentTimeline::AttachmentTimeline(size_t frameCount, int slotIndex) : Timeline(frameCount, 1),
																		   _slotIndex(slotIndex) {
	PropertyId ids[] = {((PropertyId) Property_Attachment << 32) | slotIndex};
	setPropertyIds(ids, 1);

	_attachmentNames.ensureCapacity(frameCount);
	for (size_t i = 0; i < frameCount; ++i) {
		_attachmentNames.add(String());
	}
}

AttachmentTimeline::~AttachmentTimeline() {}

void AttachmentTimeline::setAttachment(Skeleton &skeleton, Slot &slot, String *attachmentName) {
	slot.setAttachment(attachmentName == NULL || attachmentName->isEmpty() ? NULL : skeleton.getAttachment(_slotIndex, *attachmentName));
}

void AttachmentTimeline::apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha,
							   MixBlend blend, MixDirection direction) {
	SP_UNUSED(lastTime);
	SP_UNUSED(pEvents);
	SP_UNUSED(alpha);

	Slot *slot = skeleton._slots[_slotIndex];
	if (!slot->_bone._active) return;

	if (direction == MixDirection_Out) {
		if (blend == MixBlend_Setup) setAttachment(skeleton, *slot, &slot->_data._attachmentName);
		return;
	}

	if (time < _frames[0]) {
		if (blend == MixBlend_Setup || blend == MixBlend_First) {
			setAttachment(skeleton, *slot, &slot->_data._attachmentName);
		}
		return;
	}

	if (time < _frames[0]) {
		if (blend == MixBlend_Setup || blend == MixBlend_First)
			setAttachment(skeleton, *slot, &slot->_data._attachmentName);
		return;
	}

	setAttachment(skeleton, *slot, &_attachmentNames[Animation::search(_frames, time)]);
}

void AttachmentTimeline::setFrame(int frame, float time, const String &attachmentName) {
	_frames[frame] = time;
	_attachmentNames[frame] = attachmentName;
}

Vector<String> &AttachmentTimeline::getAttachmentNames() {
	return _attachmentNames;
}
