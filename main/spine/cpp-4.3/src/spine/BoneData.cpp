#include <spine/BoneData.h>
#include <spine/BonePose.h>

#include <assert.h>

using namespace spine;

BoneData::BoneData(int index, const String &name, BoneData *parent)
	: PosedDataGeneric<BonePose>(name), _index(index), _parent(parent), _length(0), _color(0.61f, 0.61f, 0.61f, 1.0f), _icon(), _iconSize(1),
	  _iconRotation(0), _visible(true) {
	assert(index >= 0);
}

int BoneData::getIndex() {
	return _index;
}

BoneData *BoneData::getParent() {
	return _parent;
}

float BoneData::getLength() {
	return _length;
}

void BoneData::setLength(float inValue) {
	_length = inValue;
}

Color &BoneData::getColor() {
	return _color;
}

const String &BoneData::getIcon() {
	return _icon;
}

void BoneData::setIcon(const String &icon) {
	this->_icon = icon;
}

float BoneData::getIconSize() {
	return _iconSize;
}

void BoneData::setIconSize(float iconSize) {
	_iconSize = iconSize;
}

float BoneData::getIconRotation() {
	return _iconRotation;
}

void BoneData::setIconRotation(float iconRotation) {
	_iconRotation = iconRotation;
}

bool BoneData::getVisible() {
	return _visible;
}

void BoneData::setVisible(bool inValue) {
	this->_visible = inValue;
}
