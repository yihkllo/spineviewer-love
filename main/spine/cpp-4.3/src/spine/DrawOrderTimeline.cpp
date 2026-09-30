#include <spine/DrawOrderTimeline.h>

#include <spine/Event.h>
#include <spine/Skeleton.h>

#include <spine/Animation.h>
#include <spine/Property.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>

using namespace spine;

RTTI_IMPL(DrawOrderTimeline, Timeline)

PropertyId DrawOrderTimeline::getPropertyId() {
	return ((PropertyId) Property_DrawOrder << 32);
}

DrawOrderTimeline::DrawOrderTimeline(size_t frameCount) : Timeline(frameCount, 1) {
	PropertyId ids[] = {getPropertyId()};
	setPropertyIds(ids, 1);
	_instant = true;

	_drawOrders.ensureCapacity(frameCount);
	for (size_t i = 0; i < frameCount; ++i) {
		Array<int> vec;
		_drawOrders.add(vec);
	}
}

void DrawOrderTimeline::apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
							  bool appliedPose) {
	SP_UNUSED(appliedPose);
	SP_UNUSED(lastTime);
	SP_UNUSED(events);
	SP_UNUSED(alpha);
	SP_UNUSED(add);

	Array<Slot *> &pose = appliedPose ? skeleton._drawOrder.getAppliedPose() : skeleton._drawOrder.getPose();
	Array<Slot *> &setup = skeleton._slots;
	if (out || time < _frames[0]) {
		if (from != MixFrom_Current) {
			pose.setSize(setup.size(), NULL);
			for (size_t i = 0, n = setup.size(); i < n; ++i) pose[i] = setup[i];
		}
		return;
	}

	Array<int> &drawOrderToSetupIndex = _drawOrders[Animation::search(_frames, time)];
	if (drawOrderToSetupIndex.size() == 0) {
		pose.setSize(setup.size(), NULL);
		for (size_t i = 0, n = setup.size(); i < n; ++i) pose[i] = setup[i];
	} else {
		pose.setSize(drawOrderToSetupIndex.size(), NULL);
		for (size_t i = 0, n = drawOrderToSetupIndex.size(); i < n; ++i) pose[i] = setup[drawOrderToSetupIndex[i]];
	}
}

void DrawOrderTimeline::setFrame(size_t frame, float time, Array<int> *drawOrder) {
	_frames[frame] = time;
	_drawOrders[frame].clear();
	if (drawOrder != NULL) {
		_drawOrders[frame].addAll(*drawOrder);
	}
}

size_t DrawOrderTimeline::getFrameCount() {
	return _frames.size();
}

Array<Array<int>> &DrawOrderTimeline::getDrawOrders() {
	return _drawOrders;
}
