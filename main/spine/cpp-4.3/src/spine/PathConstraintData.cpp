#include <spine/PathConstraintData.h>
#include <spine/PathConstraint.h>
#include <spine/BoneData.h>
#include <spine/SlotData.h>
#include <spine/Skeleton.h>

using namespace spine;

RTTI_IMPL(PathConstraintData, ConstraintData)

PathConstraintData::PathConstraintData(const String &name)
	: ConstraintDataGeneric<PathConstraint, PathConstraintPose>(name), _slot(NULL), _positionMode(PositionMode_Fixed),
	  _spacingMode(SpacingMode_Length), _rotateMode(RotateMode_Tangent), _offsetRotation(0) {
}

Array<BoneData *> &PathConstraintData::getBones() {
	return _bones;
}

SlotData &PathConstraintData::getSlot() {
	return *_slot;
}

void PathConstraintData::setSlot(SlotData &slot) {
	_slot = &slot;
}

PositionMode PathConstraintData::getPositionMode() {
	return _positionMode;
}

void PathConstraintData::setPositionMode(PositionMode positionMode) {
	_positionMode = positionMode;
}

SpacingMode PathConstraintData::getSpacingMode() {
	return _spacingMode;
}

void PathConstraintData::setSpacingMode(SpacingMode spacingMode) {
	_spacingMode = spacingMode;
}

RotateMode PathConstraintData::getRotateMode() {
	return _rotateMode;
}

void PathConstraintData::setRotateMode(RotateMode rotateMode) {
	_rotateMode = rotateMode;
}

float PathConstraintData::getOffsetRotation() {
	return _offsetRotation;
}

void PathConstraintData::setOffsetRotation(float offsetRotation) {
	_offsetRotation = offsetRotation;
}

Constraint &PathConstraintData::create(Skeleton &skeleton) {
	return *(new (__FILE__, __LINE__) PathConstraint(*this, skeleton));
}
