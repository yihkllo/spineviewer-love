#include <spine/SliderPose.h>

using namespace spine;

SliderPose::SliderPose() : _time(0), _mix(0) {
}

SliderPose::~SliderPose() {
}

void SliderPose::set(SliderPose &pose) {
	_time = pose._time;
	_mix = pose._mix;
}

float SliderPose::getTime() {
	return _time;
}

void SliderPose::setTime(float time) {
	this->_time = time;
}

float SliderPose::getMix() {
	return _mix;
}

void SliderPose::setMix(float mix) {
	this->_mix = mix;
}
