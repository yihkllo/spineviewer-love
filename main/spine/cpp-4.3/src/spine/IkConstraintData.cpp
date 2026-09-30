#include <spine/IkConstraintData.h>
#include <spine/IkConstraint.h>
#include <spine/BoneData.h>
#include <spine/Skeleton.h>

using namespace spine;

RTTI_IMPL(IkConstraintData, ConstraintData)

IkConstraintData::IkConstraintData(const String &name)
	: ConstraintDataGeneric<IkConstraint, IkConstraintPose>(name), _target(NULL), _scaleYMode(ScaleYMode_None) {
}

Array<BoneData *> &IkConstraintData::getBones() {
	return _bones;
}

BoneData &IkConstraintData::getTarget() {
	return *_target;
}

void IkConstraintData::setTarget(BoneData &inValue) {
	_target = &inValue;
}

ScaleYMode IkConstraintData::getScaleYMode() {
	return _scaleYMode;
}

void IkConstraintData::setScaleYMode(ScaleYMode scaleYMode) {
	_scaleYMode = scaleYMode;
}

Constraint &IkConstraintData::create(Skeleton &skeleton) {
	return *(new (__FILE__, __LINE__) IkConstraint(*this, skeleton));
}
