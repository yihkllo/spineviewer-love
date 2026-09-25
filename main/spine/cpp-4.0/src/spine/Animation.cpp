
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/Animation.h>
#include <spine/Event.h>
#include <spine/Skeleton.h>
#include <spine/Timeline.h>

#include <spine/ContainerUtil.h>

#include <stdint.h>

using namespace spine;

Animation::Animation(const String &name, Vector<Timeline *> &timelines, float duration) : _timelines(timelines),
																						  _timelineIds(),
																						  _duration(duration),
																						  _name(name) {
	assert(_name.length() > 0);
	for (size_t i = 0; i < timelines.size(); i++) {
		Vector<PropertyId> propertyIds = timelines[i]->getPropertyIds();
		for (size_t ii = 0; ii < propertyIds.size(); ii++)
			_timelineIds.put(propertyIds[ii], true);
	}
}

bool Animation::hasTimeline(Vector<PropertyId> ids) {
	for (size_t i = 0; i < ids.size(); i++) {
		if (_timelineIds.containsKey(ids[i])) return true;
	}
	return false;
}

Animation::~Animation() {
	ContainerUtil::cleanUpVectorOfPointers(_timelines);
}

void Animation::apply(Skeleton &skeleton, float lastTime, float time, bool loop, Vector<Event *> *pEvents, float alpha,
					  MixBlend blend, MixDirection direction) {
	if (loop && _duration != 0) {
		time = MathUtil::fmod(time, _duration);
		if (lastTime > 0) {
			lastTime = MathUtil::fmod(lastTime, _duration);
		}
	}

	for (size_t i = 0, n = _timelines.size(); i < n; ++i) {
		_timelines[i]->apply(skeleton, lastTime, time, pEvents, alpha, blend, direction);
	}
}

const String &Animation::getName() {
	return _name;
}

Vector<Timeline *> &Animation::getTimelines() {
	return _timelines;
}

float Animation::getDuration() {
	return _duration;
}

void Animation::setDuration(float inValue) {
	_duration = inValue;
}

int Animation::search(Vector<float> &frames, float target) {
	size_t n = (int) frames.size();
	for (size_t i = 1; i < n; i++) {
		if (frames[i] > target) return i - 1;
	}
	return n - 1;
}

int Animation::search(Vector<float> &frames, float target, int step) {
	size_t n = frames.size();
	for (size_t i = step; i < n; i += step)
		if (frames[i] > target) return i - step;
	return n - step;
}
