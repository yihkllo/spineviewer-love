
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/AnimationStateData.h>
#include <spine/SkeletonData.h>
#include <spine/Animation.h>

using namespace spine;

AnimationStateData::AnimationStateData(SkeletonData *skeletonData) : _skeletonData(skeletonData), _defaultMix(0) {
}

void AnimationStateData::setMix(const String &fromName, const String &toName, float duration) {
	Animation *from = _skeletonData->findAnimation(fromName);
	Animation *to = _skeletonData->findAnimation(toName);

	setMix(from, to, duration);
}

void AnimationStateData::setMix(Animation *from, Animation *to, float duration) {
	assert(from != NULL);
	assert(to != NULL);

	AnimationPair key(from, to);
	_animationToMixTime.put(key, duration);
}

float AnimationStateData::getMix(Animation *from, Animation *to) {
	assert(from != NULL);
	assert(to != NULL);

	AnimationPair key(from, to);

	if (_animationToMixTime.containsKey(key)) return _animationToMixTime[key];
	return _defaultMix;
}

SkeletonData *AnimationStateData::getSkeletonData() {
	return _skeletonData;
}

float AnimationStateData::getDefaultMix() {
	return _defaultMix;
}

void AnimationStateData::setDefaultMix(float inValue) {
	_defaultMix = inValue;
}

AnimationStateData::AnimationPair::AnimationPair(Animation *a1, Animation *a2) : _a1(a1), _a2(a2) {
}

bool AnimationStateData::AnimationPair::operator==(const AnimationPair &other) const {
	return _a1->_name == other._a1->_name && _a2->_name == other._a2->_name;
}
