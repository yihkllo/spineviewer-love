#include <spine/SlotData.h>
#include <spine/SlotPose.h>
#include <spine/BoneData.h>

#include <assert.h>

using namespace spine;

SlotData::SlotData(int index, const String &name, BoneData &boneData)
	: PosedDataGeneric<SlotPose>(name), _index(index), _boneData(boneData), _attachmentName(), _blendMode(BlendMode_Normal), _visible(true) {
	assert(index >= 0);
}

int SlotData::getIndex() {
	return _index;
}

BoneData &SlotData::getBoneData() {
	return _boneData;
}

void SlotData::setAttachmentName(const String &attachmentName) {
	_attachmentName = attachmentName;
}

const String &SlotData::getAttachmentName() {
	return _attachmentName;
}

BlendMode SlotData::getBlendMode() {
	return _blendMode;
}

void SlotData::setBlendMode(BlendMode blendMode) {
	_blendMode = blendMode;
}

bool SlotData::getVisible() {
	return _visible;
}

void SlotData::setVisible(bool visible) {
	_visible = visible;
}
