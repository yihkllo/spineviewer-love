#include <spine/Slot.h>

#include <spine/Bone.h>
#include <spine/Skeleton.h>
#include <spine/SlotData.h>
#include <spine/SlotPose.h>
#include <spine/Color.h>

using namespace spine;

Slot::Slot(SlotData &data, Skeleton &skeleton)
	: PosedGeneric<SlotData, SlotPose, SlotPose>(data), _skeleton(skeleton), _bone(*skeleton.getBones()[data._boneData._index]), _attachmentState(0) {

	if (data.getSetupPose().hasDarkColor()) {
		_pose._hasDarkColor = true;
		_constrainedPose._hasDarkColor = true;
	}
	setupPose();
}

Bone &Slot::getBone() {
	return _bone;
}

void Slot::setupPose() {
	_pose._color.set(_data._setupPose._color);
	if (_pose._hasDarkColor) _pose._darkColor.set(_data._setupPose._darkColor);
	_pose._sequenceIndex = _data._setupPose._sequenceIndex;
	if (_data._attachmentName.isEmpty())
		_pose.setAttachment(NULL);
	else {
		_pose._attachment = NULL;
		_pose.setAttachment(_skeleton.getAttachment(_data._index, _data._attachmentName));
	}
}
