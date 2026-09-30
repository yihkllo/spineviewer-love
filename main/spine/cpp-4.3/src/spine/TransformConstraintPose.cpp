#include <spine/TransformConstraintPose.h>

using namespace spine;

TransformConstraintPose::TransformConstraintPose() : _mixRotate(0), _mixX(0), _mixY(0), _mixScaleX(0), _mixScaleY(0), _mixShearY(0) {
}

TransformConstraintPose::~TransformConstraintPose() {
}

void TransformConstraintPose::set(TransformConstraintPose &pose) {
	_mixRotate = pose._mixRotate;
	_mixX = pose._mixX;
	_mixY = pose._mixY;
	_mixScaleX = pose._mixScaleX;
	_mixScaleY = pose._mixScaleY;
	_mixShearY = pose._mixShearY;
}

float TransformConstraintPose::getMixRotate() {
	return _mixRotate;
}

void TransformConstraintPose::setMixRotate(float mixRotate) {
	this->_mixRotate = mixRotate;
}

float TransformConstraintPose::getMixX() {
	return _mixX;
}

void TransformConstraintPose::setMixX(float mixX) {
	this->_mixX = mixX;
}

float TransformConstraintPose::getMixY() {
	return _mixY;
}

void TransformConstraintPose::setMixY(float mixY) {
	this->_mixY = mixY;
}

float TransformConstraintPose::getMixScaleX() {
	return _mixScaleX;
}

void TransformConstraintPose::setMixScaleX(float mixScaleX) {
	this->_mixScaleX = mixScaleX;
}

float TransformConstraintPose::getMixScaleY() {
	return _mixScaleY;
}

void TransformConstraintPose::setMixScaleY(float mixScaleY) {
	this->_mixScaleY = mixScaleY;
}

float TransformConstraintPose::getMixShearY() {
	return _mixShearY;
}

void TransformConstraintPose::setMixShearY(float mixShearY) {
	this->_mixShearY = mixShearY;
}
