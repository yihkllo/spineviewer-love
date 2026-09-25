
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/EventTimeline.h>

#include <spine/Event.h>
#include <spine/Skeleton.h>

#include <spine/Animation.h>
#include <spine/ContainerUtil.h>
#include <spine/EventData.h>
#include <spine/Property.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>

#include <float.h>

using namespace spine;

RTTI_IMPL(EventTimeline, Timeline)

EventTimeline::EventTimeline(size_t frameCount) : Timeline(frameCount, 1) {
	PropertyId ids[] = {((PropertyId) Property_Event << 32)};
	setPropertyIds(ids, 1);
	_events.setSize(frameCount, NULL);
}

EventTimeline::~EventTimeline() {
	ContainerUtil::cleanUpVectorOfPointers(_events);
}

void EventTimeline::apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha,
						  MixBlend blend, MixDirection direction) {
	if (pEvents == NULL) return;

	Vector<Event *> &events = *pEvents;

	size_t frameCount = _frames.size();

	if (lastTime > time) {
		apply(skeleton, lastTime, FLT_MAX, pEvents, alpha, blend, direction);
		lastTime = -1.0f;
	} else if (lastTime >= _frames[frameCount - 1]) {
		return;
	}

	if (time < _frames[0]) return;

	int i;
	if (lastTime < _frames[0]) {
		i = 0;
	} else {
		i = Animation::search(_frames, lastTime) + 1;
		float frameTime = _frames[i];
		while (i > 0) {
			if (_frames[i - 1] != frameTime) break;
			i--;
		}
	}

	for (; (size_t) i < frameCount && time >= _frames[i]; i++)
		events.add(_events[i]);
}

void EventTimeline::setFrame(size_t frame, Event *event) {
	_frames[frame] = event->getTime();
	_events[frame] = event;
}

Vector<Event *> &EventTimeline::getEvents() { return _events; }
