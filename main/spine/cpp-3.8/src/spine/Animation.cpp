
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/Animation.h>
#include <spine/Timeline.h>
#include <spine/Skeleton.h>
#include <spine/Event.h>

#include <spine/ContainerUtil.h>

#include <stdint.h>

using namespace spine;

Animation::Animation(const String &name, Vector<Timeline *> &timelines, float duration) :
		_timelines(timelines),
		_timelineIds(),
		_duration(duration),
		_name(name) {
	assert(_name.length() > 0);
	for (int i = 0; i < (int)timelines.size(); i++)
		_timelineIds.put(timelines[i]->getPropertyId(), true);
}

bool Animation::hasTimeline(int id) {
	return _timelineIds.containsKey(id);
}

Animation::~Animation() {
	ContainerUtil::cleanUpVectorOfPointers(_timelines);
}

void Animation::apply(Skeleton &skeleton, float lastTime, float time, bool loop, Vector<Event *> *pEvents, float alpha,
	MixBlend blend, MixDirection direction
) {
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

int Animation::binarySearch(Vector<float> &values, float target, int step) {
	int low = 0;
	int size = (int)values.size();
	int high = size / step - 2;
	if (high == 0) {
		return step;
	}

	int current = (int) (static_cast<uint32_t>(high) >> 1);
	while (true) {
		if (values[(current + 1) * step] <= target)
			low = current + 1;
		else
			high = current;

		if (low == high) return (low + 1) * step;

		current = (int) (static_cast<uint32_t>(low + high) >> 1);
	}
}

int Animation::binarySearch(Vector<float> &values, float target) {
	int low = 0;
	int size = (int)values.size();
	int high = size - 2;
	if (high == 0) return 1;

	int current = (int) (static_cast<uint32_t>(high) >> 1);
	while (true) {
		if (values[(current + 1)] <= target)
			low = current + 1;
		else
			high = current;

		if (low == high) return (low + 1);

		current = (int) (static_cast<uint32_t>(low + high) >> 1);
	}
}

int Animation::linearSearch(Vector<float> &values, float target, int step) {
	for (int i = 0, last = (int)values.size() - step; i <= last; i += step) {
		if (values[i] > target) {
			return i;
		}
	}

	return -1;
}
