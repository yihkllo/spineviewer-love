#include <spine/SliderData.h>
#include <spine/Slider.h>
#include <spine/Skeleton.h>
#include <spine/ArrayUtils.h>
#include <spine/TransformConstraintData.h>

using namespace spine;

RTTI_IMPL(SliderData, ConstraintData)

SliderData::SliderData(const String &name)
	: ConstraintDataGeneric<Slider, SliderPose>(name), _animation(NULL), _additive(false), _loop(false), _bone(NULL), _property(NULL), _offset(0.0f),
	  _scale(0.0f), _max(0.0f), _local(false) {
}

SliderData::~SliderData() {
	if (_property) {
		ArrayUtils::deleteElements(_property->_to);
		delete _property;
		_property = NULL;
	}
}

Constraint &SliderData::create(Skeleton &skeleton) {
	return *(new (__FILE__, __LINE__) Slider(*this, skeleton));
}

Animation &SliderData::getAnimation() {
	return *_animation;
}

void SliderData::setAnimation(Animation &animation) {
	_animation = &animation;
}

bool SliderData::getAdditive() {
	return _additive;
}

void SliderData::setAdditive(bool additive) {
	_additive = additive;
}

bool SliderData::getLoop() {
	return _loop;
}

void SliderData::setLoop(bool loop) {
	_loop = loop;
}

BoneData *SliderData::getBone() {
	return _bone;
}

void SliderData::setBone(BoneData *bone) {
	_bone = bone;
}

FromProperty *SliderData::getProperty() {
	return _property;
}

void SliderData::setProperty(FromProperty *property) {
	_property = property;
}

float SliderData::getScale() {
	return _scale;
}

void SliderData::setScale(float scale) {
	_scale = scale;
}

float SliderData::getOffset() {
	return _offset;
}

void SliderData::setOffset(float offset) {
	_offset = offset;
}

float SliderData::getMax() {
	return _max;
}

void SliderData::setMax(float max) {
	_max = max;
}

bool SliderData::getLocal() {
	return _local;
}

void SliderData::setLocal(bool local) {
	_local = local;
}
