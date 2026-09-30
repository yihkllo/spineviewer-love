#include <spine/IkConstraintPose.h>

using namespace spine;

IkConstraintPose::IkConstraintPose() : _bendDirection(0), _compress(false), _stretch(false), _mix(0), _softness(0) {
}

IkConstraintPose::~IkConstraintPose() {
}

void IkConstraintPose::set(IkConstraintPose &pose) {
	_mix = pose._mix;
	_softness = pose._softness;
	_bendDirection = pose._bendDirection;
	_compress = pose._compress;
	_stretch = pose._stretch;
}

float IkConstraintPose::getMix() {
	return _mix;
}

void IkConstraintPose::setMix(float mix) {
	_mix = mix;
}

float IkConstraintPose::getSoftness() {
	return _softness;
}

void IkConstraintPose::setSoftness(float softness) {
	_softness = softness;
}

int IkConstraintPose::getBendDirection() {
	return _bendDirection;
}

void IkConstraintPose::setBendDirection(int bendDirection) {
	_bendDirection = bendDirection;
}

bool IkConstraintPose::getCompress() {
	return _compress;
}

void IkConstraintPose::setCompress(bool compress) {
	_compress = compress;
}

bool IkConstraintPose::getStretch() {
	return _stretch;
}

void IkConstraintPose::setStretch(bool stretch) {
	_stretch = stretch;
}
