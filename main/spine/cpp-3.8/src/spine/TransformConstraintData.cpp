
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/TransformConstraintData.h>

#include <spine/BoneData.h>

#include <assert.h>

using namespace spine;
TransformConstraintData::TransformConstraintData(const String &name) :
		ConstraintData(name),
		_target(NULL),
		_rotateMix(0),
		_translateMix(0),
		_scaleMix(0),
		_shearMix(0),
		_offsetRotation(0),
		_offsetX(0),
		_offsetY(0),
		_offsetScaleX(0),
		_offsetScaleY(0),
		_offsetShearY(0),
		_relative(false),
		_local(false) {
}

Vector<BoneData *> &TransformConstraintData::getBones() {
	return _bones;
}

BoneData *TransformConstraintData::getTarget() {
	return _target;
}

float TransformConstraintData::getRotateMix() {
	return _rotateMix;
}

float TransformConstraintData::getTranslateMix() {
	return _translateMix;
}

float TransformConstraintData::getScaleMix() {
	return _scaleMix;
}

float TransformConstraintData::getShearMix() {
	return _shearMix;
}

float TransformConstraintData::getOffsetRotation() {
	return _offsetRotation;
}

float TransformConstraintData::getOffsetX() {
	return _offsetX;
}

float TransformConstraintData::getOffsetY() {
	return _offsetY;
}

float TransformConstraintData::getOffsetScaleX() {
	return _offsetScaleX;
}

float TransformConstraintData::getOffsetScaleY() {
	return _offsetScaleY;
}

float TransformConstraintData::getOffsetShearY() {
	return _offsetShearY;
}

bool TransformConstraintData::isRelative() {
	return _relative;
}

bool TransformConstraintData::isLocal() {
	return _local;
}
