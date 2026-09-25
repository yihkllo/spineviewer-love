
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/SlotData.h>

#include <assert.h>

using namespace spine;

SlotData::SlotData(int index, const String &name, BoneData &boneData) : _index(index),
																		_name(name),
																		_boneData(boneData),
																		_color(1, 1, 1, 1),
																		_darkColor(0, 0, 0, 0),
																		_hasDarkColor(false),
																		_attachmentName(),
																		_blendMode(BlendMode_Normal) {
	assert(_index >= 0);
	assert(_name.length() > 0);
}

int SlotData::getIndex() {
	return _index;
}

const String &SlotData::getName() {
	return _name;
}

BoneData &SlotData::getBoneData() {
	return _boneData;
}

Color &SlotData::getColor() {
	return _color;
}

Color &SlotData::getDarkColor() {
	return _darkColor;
}

bool SlotData::hasDarkColor() {
	return _hasDarkColor;
}

void SlotData::setHasDarkColor(bool inValue) {
	_hasDarkColor = inValue;
}

const String &SlotData::getAttachmentName() {
	return _attachmentName;
}

void SlotData::setAttachmentName(const String &inValue) {
	_attachmentName = inValue;
}

BlendMode SlotData::getBlendMode() {
	return _blendMode;
}

void SlotData::setBlendMode(BlendMode inValue) {
	_blendMode = inValue;
}
