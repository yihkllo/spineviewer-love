
#include <spine/TransformConstraintData.h>

#include <spine/BoneData.h>

#include <assert.h>

using namespace spine;

RTTI_IMPL(TransformConstraintData, ConstraintData)

TransformConstraintData::TransformConstraintData(const String &name) : ConstraintData(name),
																	   _target(NULL),
																	   _mixRotate(0),
																	   _mixX(0),
																	   _mixY(0),
																	   _mixScaleX(0),
																	   _mixScaleY(0),
																	   _mixShearY(0),
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

float TransformConstraintData::getMixRotate() {
	return _mixRotate;
}

float TransformConstraintData::getMixX() {
	return _mixX;
}

float TransformConstraintData::getMixY() {
	return _mixY;
}

float TransformConstraintData::getMixScaleX() {
	return _mixScaleX;
}

float TransformConstraintData::getMixScaleY() {
	return _mixScaleY;
}

float TransformConstraintData::getMixShearY() {
	return _mixShearY;
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

void TransformConstraintData::setTarget(BoneData *target) {
	_target = target;
}

void TransformConstraintData::setMixRotate(float mixRotate) {
	_mixRotate = mixRotate;
}

void TransformConstraintData::setMixX(float mixX) {
	_mixX = mixX;
}

void TransformConstraintData::setMixY(float mixY) {
	_mixY = mixY;
}

void TransformConstraintData::setMixScaleX(float mixScaleX) {
	_mixScaleX = mixScaleX;
}

void TransformConstraintData::setMixScaleY(float mixScaleY) {
	_mixScaleY = mixScaleY;
}

void TransformConstraintData::setMixShearY(float mixShearY) {
	_mixShearY = mixShearY;
}

void TransformConstraintData::setOffsetRotation(float offsetRotation) {
	_offsetRotation = offsetRotation;
}

void TransformConstraintData::setOffsetX(float offsetX) {
	_offsetX = offsetX;
}

void TransformConstraintData::setOffsetY(float offsetY) {
	_offsetY = offsetY;
}

void TransformConstraintData::setOffsetScaleX(float offsetScaleX) {
	_offsetScaleX = offsetScaleX;
}

void TransformConstraintData::setOffsetScaleY(float offsetScaleY) {
	_offsetScaleY = offsetScaleY;
}

void TransformConstraintData::setOffsetShearY(float offsetShearY) {
	_offsetShearY = offsetShearY;
}

void TransformConstraintData::setRelative(bool isRelative) {
	_relative = isRelative;
}

void TransformConstraintData::setLocal(bool isLocal) {
	_local = isLocal;
}
