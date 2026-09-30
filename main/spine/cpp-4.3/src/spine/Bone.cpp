#include <spine/Bone.h>
#include <spine/BoneData.h>
#include <spine/BonePose.h>

using namespace spine;

RTTI_IMPL(Bone, Update)

bool Bone::yDown = true;

Bone::Bone(BoneData &data, Bone *parent)
	: PosedGeneric<BoneData, BonePose, BonePose>(data), PosedActive(), _parent(parent), _children(), _sorted(false) {
	_constrainedPose._bone = this;
	_appliedPose->_bone = this;
}

Bone::Bone(Bone &bone, Bone *parent)
	: PosedGeneric<BoneData, BonePose, BonePose>(bone._data), PosedActive(), _parent(parent), _children(), _sorted(false) {
	_constrainedPose._bone = this;
	_appliedPose->_bone = this;
	_pose.set(bone._pose);
}

Bone *Bone::getParent() {
	return _parent;
}

Array<Bone *> &Bone::getChildren() {
	return _children;
}
