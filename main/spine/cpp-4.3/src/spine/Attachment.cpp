#include <spine/Attachment.h>

#include <spine/Bone.h>
#include <spine/Slot.h>

#include <assert.h>

using namespace spine;

RTTI_IMPL_NOPARENT(Attachment)

Attachment::Attachment(const String &name) : _name(name), _timelineAttachment(this), _refCount(0) {
	assert(_name.length() > 0);
}

Attachment::~Attachment() {
}

const String &Attachment::getName() const {
	return _name;
}

Attachment *Attachment::getTimelineAttachment() {
	return _timelineAttachment;
}

void Attachment::setTimelineAttachment(Attachment *attachment) {
	_timelineAttachment = attachment;
}

Array<int> &Attachment::getTimelineSlots() {
	return _timelineSlots;
}

void Attachment::setTimelineSlots(Array<int> &timelineSlots) {
	_timelineSlots.clearAndAddAll(timelineSlots);
}

bool Attachment::isTimelineActive(Array<Slot *> &slots, int slotIndex, bool appliedPose) {
	Slot *slot = slots[slotIndex];
	if (slot->getBone().isActive()) {
		Attachment *attachment = appliedPose ? slot->getAppliedPose().getAttachment() : slot->getPose().getAttachment();
		if (attachment != NULL && attachment->getTimelineAttachment() == this) return true;
	}
	for (size_t i = 0, n = _timelineSlots.size(); i < n; ++i) {
		slot = slots[_timelineSlots[i]];
		if (!slot->getBone().isActive()) continue;
		Attachment *attachment = appliedPose ? slot->getAppliedPose().getAttachment() : slot->getPose().getAttachment();
		if (attachment != NULL && attachment->getTimelineAttachment() == this) return true;
	}
	return false;
}

int Attachment::getRefCount() {
	return _refCount;
}

void Attachment::reference() {
	_refCount++;
}

void Attachment::dereference() {
	_refCount--;
}
