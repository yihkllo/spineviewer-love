#include <spine/SkeletonData.h>

#include <spine/Animation.h>
#include <spine/BoneData.h>
#include <spine/ConstraintData.h>
#include <spine/EventData.h>
#include <spine/IkConstraintData.h>
#include <spine/PathConstraintData.h>
#include <spine/PhysicsConstraintData.h>
#include <spine/Skin.h>
#include <spine/SlotData.h>
#include <spine/TransformConstraintData.h>
#include <spine/SliderData.h>

#include <spine/ArrayUtils.h>

using namespace spine;

SkeletonData::SkeletonData()
	: _name(), _defaultSkin(NULL), _x(0), _y(0), _width(0), _height(0), _referenceScale(100), _version(), _hash(), _fps(30), _imagesPath(),
	  _audioPath() {
}

SkeletonData::~SkeletonData() {
	ArrayUtils::deleteElements(_bones);
	ArrayUtils::deleteElements(_slots);
	ArrayUtils::deleteElements(_skins);

	_defaultSkin = NULL;

	ArrayUtils::deleteElements(_events);
	ArrayUtils::deleteElements(_animations);
	ArrayUtils::deleteElements(_constraints);
	for (size_t i = 0; i < _strings.size(); i++) {
		SpineExtension::free(_strings[i], __FILE__, __LINE__);
	}
}

BoneData *SkeletonData::findBone(const String &boneName) {
	return ArrayUtils::findWithName(_bones, boneName);
}

SlotData *SkeletonData::findSlot(const String &slotName) {
	return ArrayUtils::findWithName(_slots, slotName);
}

Skin *SkeletonData::findSkin(const String &skinName) {
	return ArrayUtils::findWithName(_skins, skinName);
}

EventData *SkeletonData::findEvent(const String &eventDataName) {
	return ArrayUtils::findWithName(_events, eventDataName);
}

Animation *SkeletonData::findAnimation(const String &animationName) {
	return ArrayUtils::findWithName(_animations, animationName);
}

Array<Animation *> &SkeletonData::findSliderAnimations(Array<Animation *> &animations) {
	for (size_t i = 0, n = _constraints.size(); i < n; i++) {
		ConstraintData *constraint = _constraints[i];
		if (constraint->getRTTI().instanceOf(SliderData::rtti)) {
			SliderData *data = static_cast<SliderData *>(constraint);
			if (data->_animation != NULL) animations.add(data->_animation);
		}
	}
	return animations;
}

const String &SkeletonData::getName() {
	return _name;
}

void SkeletonData::setName(const String &inValue) {
	_name = inValue;
}

Array<BoneData *> &SkeletonData::getBones() {
	return _bones;
}

Array<SlotData *> &SkeletonData::getSlots() {
	return _slots;
}

Array<Skin *> &SkeletonData::getSkins() {
	return _skins;
}

Skin *SkeletonData::getDefaultSkin() {
	return _defaultSkin;
}

void SkeletonData::setDefaultSkin(Skin *inValue) {
	_defaultSkin = inValue;
}

Array<EventData *> &SkeletonData::getEvents() {
	return _events;
}

Array<Animation *> &SkeletonData::getAnimations() {
	return _animations;
}

float SkeletonData::getX() {
	return _x;
}

void SkeletonData::setX(float inValue) {
	_x = inValue;
}

float SkeletonData::getY() {
	return _y;
}

void SkeletonData::setY(float inValue) {
	_y = inValue;
}

float SkeletonData::getWidth() {
	return _width;
}

void SkeletonData::setWidth(float inValue) {
	_width = inValue;
}

float SkeletonData::getHeight() {
	return _height;
}

void SkeletonData::setHeight(float inValue) {
	_height = inValue;
}

float SkeletonData::getReferenceScale() {
	return _referenceScale;
}

void SkeletonData::setReferenceScale(float inValue) {
	_referenceScale = inValue;
}

const String &SkeletonData::getVersion() {
	return _version;
}

void SkeletonData::setVersion(const String &inValue) {
	_version = inValue;
}

const String &SkeletonData::getHash() {
	return _hash;
}

void SkeletonData::setHash(const String &inValue) {
	_hash = inValue;
}

const String &SkeletonData::getImagesPath() {
	return _imagesPath;
}

void SkeletonData::setImagesPath(const String &inValue) {
	_imagesPath = inValue;
}

const String &SkeletonData::getAudioPath() {
	return _audioPath;
}

void SkeletonData::setAudioPath(const String &inValue) {
	_audioPath = inValue;
}

float SkeletonData::getFps() {
	return _fps;
}

void SkeletonData::setFps(float inValue) {
	_fps = inValue;
}

Array<ConstraintData *> &SkeletonData::getConstraints() {
	return _constraints;
}
