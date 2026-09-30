#include <spine/EventTimeline.h>

#include <spine/Event.h>
#include <spine/Skeleton.h>

#include <spine/Animation.h>
#include <spine/ArrayUtils.h>
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
	_instant = true;
}

EventTimeline::~EventTimeline() {
	ArrayUtils::deleteElements(_events);
}

void EventTimeline::apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *pEvents, float alpha, MixFrom from, bool add, bool out,
						  bool appliedPose) {
	SP_UNUSED(skeleton);
	if (pEvents == NULL) return;

	Array<Event *> &events = *pEvents;

	size_t frameCount = _frames.size();

	if (lastTime > time) {

		apply(skeleton, lastTime, FLT_MAX, pEvents, 0, MixFrom_Current, false, false, false);
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

	for (; (size_t) i < frameCount && time >= _frames[i]; i++) events.add(_events[i]);
}

void EventTimeline::setFrame(size_t frame, Event &event) {
	_frames[frame] = event.getTime();
	_events[frame] = &event;
}

size_t EventTimeline::getFrameCount() {
	return _frames.size();
}

Array<Event *> &EventTimeline::getEvents() {
	return _events;
}
