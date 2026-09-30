#include <spine/PathConstraintPose.h>

using namespace spine;

PathConstraintPose::PathConstraintPose() : Pose<PathConstraintPose>(), _position(0), _spacing(0), _mixRotate(0), _mixX(0), _mixY(0) {
}

PathConstraintPose::~PathConstraintPose() {
}

void PathConstraintPose::set(PathConstraintPose &pose) {
	_position = pose._position;
	_spacing = pose._spacing;
	_mixRotate = pose._mixRotate;
	_mixX = pose._mixX;
	_mixY = pose._mixY;
}

float PathConstraintPose::getPosition() {
	return _position;
}

void PathConstraintPose::setPosition(float position) {
	_position = position;
}

float PathConstraintPose::getSpacing() {
	return _spacing;
}

void PathConstraintPose::setSpacing(float spacing) {
	_spacing = spacing;
}

float PathConstraintPose::getMixRotate() {
	return _mixRotate;
}

void PathConstraintPose::setMixRotate(float mixRotate) {
	_mixRotate = mixRotate;
}

float PathConstraintPose::getMixX() {
	return _mixX;
}

void PathConstraintPose::setMixX(float mixX) {
	_mixX = mixX;
}

float PathConstraintPose::getMixY() {
	return _mixY;
}

void PathConstraintPose::setMixY(float mixY) {
	_mixY = mixY;
}
