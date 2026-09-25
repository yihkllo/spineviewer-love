
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/Slot.h>

#include <spine/SlotData.h>
#include <spine/Bone.h>
#include <spine/Skeleton.h>
#include <spine/Attachment.h>

using namespace spine;

Slot::Slot(SlotData &data, Bone &bone) :
		_data(data),
		_bone(bone),
		_skeleton(bone.getSkeleton()),
		_color(1, 1, 1, 1),
		_darkColor(0, 0, 0, 0),
		_hasDarkColor(data.hasDarkColor()),
		_attachment(NULL),
		_attachmentState(0),
		_attachmentTime(0) {
	setToSetupPose();
}

void Slot::setToSetupPose() {
	_color.set(_data.getColor());
	if  (_hasDarkColor) _darkColor.set(_data.getDarkColor());

	const String &attachmentName = _data.getAttachmentName();
	if (attachmentName.length() > 0) {
		_attachment = NULL;
		setAttachment(_skeleton.getAttachment(_data.getIndex(), attachmentName));
	} else {
		setAttachment(NULL);
	}
}

SlotData &Slot::getData() {
	return _data;
}

Bone &Slot::getBone() {
	return _bone;
}

Skeleton &Slot::getSkeleton() {
	return _skeleton;
}

Color &Slot::getColor() {
	return _color;
}

Color &Slot::getDarkColor() {
	return _darkColor;
}

bool Slot::hasDarkColor() {
	return _hasDarkColor;
}

Attachment *Slot::getAttachment() {
	return _attachment;
}

void Slot::setAttachment(Attachment *inValue) {
	if (_attachment == inValue) {
		return;
	}

	_attachment = inValue;
	_attachmentTime = _skeleton.getTime();
	_deform.clear();
}

int Slot::getAttachmentState() {
    return _attachmentState;
}

void Slot::setAttachmentState(int state) {
    _attachmentState = state;
}

float Slot::getAttachmentTime() {
	return _skeleton.getTime() - _attachmentTime;
}

void Slot::setAttachmentTime(float inValue) {
	_attachmentTime = _skeleton.getTime() - inValue;
}

Vector<float> &Slot::getDeform() {
	return _deform;
}
