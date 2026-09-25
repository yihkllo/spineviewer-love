
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/PathConstraintData.h>

#include <spine/BoneData.h>
#include <spine/SlotData.h>

#include <assert.h>

using namespace spine;

PathConstraintData::PathConstraintData(const String &name) :
		ConstraintData(name),
		_target(NULL),
		_positionMode(PositionMode_Fixed),
		_spacingMode(SpacingMode_Length),
		_rotateMode(RotateMode_Tangent),
		_offsetRotation(0),
		_position(0),
		_spacing(0),
		_rotateMix(0),
		_translateMix(0) {
}

Vector<BoneData *> &PathConstraintData::getBones() {
	return _bones;
}

SlotData *PathConstraintData::getTarget() {
	return _target;
}

void PathConstraintData::setTarget(SlotData *inValue) {
	_target = inValue;
}

PositionMode PathConstraintData::getPositionMode() {
	return _positionMode;
}

void PathConstraintData::setPositionMode(PositionMode inValue) {
	_positionMode = inValue;
}

SpacingMode PathConstraintData::getSpacingMode() {
	return _spacingMode;
}

void PathConstraintData::setSpacingMode(SpacingMode inValue) {
	_spacingMode = inValue;
}

RotateMode PathConstraintData::getRotateMode() {
	return _rotateMode;
}

void PathConstraintData::setRotateMode(RotateMode inValue) {
	_rotateMode = inValue;
}

float PathConstraintData::getOffsetRotation() {
	return _offsetRotation;
}

void PathConstraintData::setOffsetRotation(float inValue) {
	_offsetRotation = inValue;
}

float PathConstraintData::getPosition() {
	return _position;
}

void PathConstraintData::setPosition(float inValue) {
	_position = inValue;
}

float PathConstraintData::getSpacing() {
	return _spacing;
}

void PathConstraintData::setSpacing(float inValue) {
	_spacing = inValue;
}

float PathConstraintData::getRotateMix() {
	return _rotateMix;
}

void PathConstraintData::setRotateMix(float inValue) {
	_rotateMix = inValue;
}

float PathConstraintData::getTranslateMix() {
	return _translateMix;
}

void PathConstraintData::setTranslateMix(float inValue) {
	_translateMix = inValue;
}
