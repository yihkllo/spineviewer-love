#include <spine/SlotPose.h>
#include <spine/Attachment.h>
#include <spine/VertexAttachment.h>

using namespace spine;

SlotPose::SlotPose() : _color(1, 1, 1, 1), _darkColor(0, 0, 0, 0), _hasDarkColor(false), _attachment(nullptr), _sequenceIndex(0) {
}

SlotPose::~SlotPose() {
}

void SlotPose::set(SlotPose &pose) {
	_color.set(pose._color);
	if (pose._hasDarkColor) _darkColor.set(pose._darkColor);
	_hasDarkColor = pose._hasDarkColor;
	_attachment = pose._attachment;
	_sequenceIndex = pose._sequenceIndex;
	_deform.clear();
	_deform.addAll(pose._deform);
}

Color &SlotPose::getColor() {
	return _color;
}

Color &SlotPose::getDarkColor() {
	return _darkColor;
}

bool SlotPose::hasDarkColor() {
	return _hasDarkColor;
}

void SlotPose::setHasDarkColor(bool hasDarkColor) {
	_hasDarkColor = hasDarkColor;
}

Attachment *SlotPose::getAttachment() {
	return _attachment;
}

void SlotPose::setAttachment(Attachment *attachment) {
	if (_attachment == attachment) return;

	if (!attachment || !_attachment || attachment->getTimelineAttachment() != _attachment->getTimelineAttachment()) _deform.clear();
	_attachment = attachment;
	_sequenceIndex = -1;
}

int SlotPose::getSequenceIndex() {
	return _sequenceIndex;
}

void SlotPose::setSequenceIndex(int sequenceIndex) {
	_sequenceIndex = sequenceIndex;
}

Array<float> &SlotPose::getDeform() {
	return _deform;
}
