#include <spine/Animation.h>
#include <spine/Event.h>
#include <spine/Skeleton.h>
#include <spine/Timeline.h>
#include <spine/BoneTimeline.h>
#include <spine/RotateTimeline.h>
#include <spine/TranslateTimeline.h>

#include <spine/ArrayUtils.h>

#include <stdint.h>

using namespace spine;

Animation::Animation(const String &name) : _timelines(), _timelineIds(), _bones(), _duration(0), _name(name), _color(1, 1, 1, 1) {
}

bool Animation::hasTimeline(Array<PropertyId> &ids) {
	for (size_t i = 0; i < ids.size(); i++) {
		if (_timelineIds.containsKey(ids[i])) return true;
	}
	return false;
}

Animation::~Animation() {
	ArrayUtils::deleteElements(_timelines);
}

void Animation::apply(Skeleton &skeleton, float lastTime, float time, bool loop, Array<Event *> *events, float alpha, MixFrom from, bool add,
					  bool out, bool appliedPose) {
	if (loop && _duration != 0) {
		time = MathUtil::fmod(time, _duration);
		if (lastTime > 0) {
			lastTime = MathUtil::fmod(lastTime, _duration);
		}
	}

	for (size_t i = 0, n = _timelines.size(); i < n; ++i) {
		_timelines[i]->apply(skeleton, lastTime, time, events, alpha, from, add, out, appliedPose);
	}
}

const String &Animation::getName() {
	return _name;
}

const Array<int> &Animation::getBones() {
	return _bones;
}

Color &Animation::getColor() {
	return _color;
}

Array<Timeline *> &Animation::getTimelines() {
	return _timelines;
}

float Animation::getDuration() {
	return _duration;
}

void Animation::setDuration(float inValue) {
	_duration = inValue;
}

int Animation::search(Array<float> &frames, float target) {
	size_t n = (int) frames.size();
	for (size_t i = 1; i < n; i++) {
		if (frames[i] > target) return (int) (i - 1);
	}
	return (int) (n - 1);
}

int Animation::search(Array<float> &frames, float target, int step) {
	size_t n = frames.size();
	for (size_t i = step; i < n; i += step)
		if (frames[i] > target) return (int) (i - step);
	return (int) (n - step);
}

void Animation::setTimelines(Array<Timeline *> &timelines, Array<int> &bones) {
	_timelines = timelines;
	_bones = bones;

	size_t n = timelines.size();
	_timelineIds.clear();
	for (size_t i = 0; i < n; i++) {
		Timeline *timeline = timelines[i];
		Array<PropertyId> &propertyIds = timeline->getPropertyIds();
		for (size_t ii = 0; ii < propertyIds.size(); ii++) {
			_timelineIds.put(propertyIds[ii], true);
		}
	}
}
