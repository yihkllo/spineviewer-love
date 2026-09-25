
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/AttachmentTimeline.h>

#include <spine/Skeleton.h>
#include <spine/Event.h>

#include <spine/Animation.h>
#include <spine/Bone.h>
#include <spine/TimelineType.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>

using namespace spine;

RTTI_IMPL(AttachmentTimeline, Timeline)

AttachmentTimeline::AttachmentTimeline(int frameCount) : Timeline(), _slotIndex(0) {
	_frames.ensureCapacity(frameCount);
	_attachmentNames.ensureCapacity(frameCount);

	_frames.setSize(frameCount, 0);

	for (int i = 0; i < frameCount; ++i) {
		_attachmentNames.add(String());
	}
}

void AttachmentTimeline::setAttachment(Skeleton& skeleton, Slot& slot, String* attachmentName) {
    slot.setAttachment(attachmentName == NULL || attachmentName->isEmpty() ? NULL : skeleton.getAttachment(_slotIndex, *attachmentName));
}

void AttachmentTimeline::apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha,
	MixBlend blend, MixDirection direction
) {
	SP_UNUSED(lastTime);
	SP_UNUSED(pEvents);
	SP_UNUSED(alpha);

	assert(_slotIndex < skeleton._slots.size());

	String *attachmentName;
	Slot *slotP = skeleton._slots[_slotIndex];
	Slot &slot = *slotP;
	if (!slot._bone.isActive()) return;

	if (direction == MixDirection_Out) {
	    if (blend == MixBlend_Setup) setAttachment(skeleton, slot, &slot._data._attachmentName);
		return;
	}

	if (time < _frames[0]) {
		if (blend == MixBlend_Setup || blend == MixBlend_First) {
            setAttachment(skeleton, slot, &slot._data._attachmentName);
		}
		return;
	}

	size_t frameIndex;
	if (time >= _frames[_frames.size() - 1]) {
		frameIndex = _frames.size() - 1;
	} else {
		frameIndex = Animation::binarySearch(_frames, time, 1) - 1;
	}

	attachmentName = &_attachmentNames[frameIndex];
	slot.setAttachment(attachmentName->length() == 0 ? NULL : skeleton.getAttachment(_slotIndex, *attachmentName));
}

int AttachmentTimeline::getPropertyId() {
	return ((int) TimelineType_Attachment << 24) + _slotIndex;
}

void AttachmentTimeline::setFrame(int frameIndex, float time, const String &attachmentName) {
	_frames[frameIndex] = time;
	_attachmentNames[frameIndex] = attachmentName;
}

size_t AttachmentTimeline::getSlotIndex() {
	return _slotIndex;
}

void AttachmentTimeline::setSlotIndex(size_t inValue) {
	_slotIndex = inValue;
}

Vector<float> &AttachmentTimeline::getFrames() {
	return _frames;
}

Vector<String> &AttachmentTimeline::getAttachmentNames() {
	return _attachmentNames;
}

size_t AttachmentTimeline::getFrameCount() {
	return _frames.size();
}
