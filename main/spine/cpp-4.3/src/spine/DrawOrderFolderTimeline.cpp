#include <spine/DrawOrderFolderTimeline.h>

#include <spine/Animation.h>
#include <spine/Event.h>
#include <spine/Property.h>
#include <spine/Skeleton.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>

using namespace spine;

RTTI_IMPL(DrawOrderFolderTimeline, Timeline)

DrawOrderFolderTimeline::DrawOrderFolderTimeline(size_t frameCount, Array<int> &slots, size_t slotCount) : Timeline(frameCount, 1) {
	Array<PropertyId> ids(slots.size());
	for (size_t i = 0; i < slots.size(); ++i) ids.add(((PropertyId) Property_DrawOrderFolder << 32) | (PropertyId) slots[i]);
	setPropertyIds(ids.buffer(), ids.size());

	_slots.addAll(slots);
	_drawOrders.ensureCapacity(frameCount);
	_inFolder.setSize(slotCount, false);
	for (size_t i = 0; i < _slots.size(); ++i) _inFolder[_slots[i]] = true;
	_instant = true;
	for (size_t i = 0; i < frameCount; ++i) {
		Array<int> vec;
		_drawOrders.add(vec);
	}
}

void DrawOrderFolderTimeline::apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add,
									bool out, bool appliedPose) {
	SP_UNUSED(lastTime);
	SP_UNUSED(events);
	SP_UNUSED(alpha);
	SP_UNUSED(add);
	Array<Slot *> &pose = appliedPose ? skeleton._drawOrder.getAppliedPose() : skeleton._drawOrder.getPose();
	Array<Slot *> &setupPose = skeleton._slots;

	if (out || time < _frames[0]) {
		if (from != MixFrom_Current) setup(pose, setupPose);
	} else {
		Array<int> &drawOrder = _drawOrders[Animation::search(_frames, time)];
		if (drawOrder.size() == 0)
			setup(pose, setupPose);
		else
			apply(pose, setupPose, drawOrder);
	}
}

size_t DrawOrderFolderTimeline::getFrameCount() {
	return _frames.size();
}

Array<int> &DrawOrderFolderTimeline::getSlots() {
	return _slots;
}

Array<Array<int>> &DrawOrderFolderTimeline::getDrawOrders() {
	return _drawOrders;
}

void DrawOrderFolderTimeline::setFrame(size_t frame, float time, Array<int> *drawOrder) {
	_frames[frame] = time;
	_drawOrders[frame].clear();
	if (drawOrder != NULL) _drawOrders[frame].addAll(*drawOrder);
}

void DrawOrderFolderTimeline::setup(Array<Slot *> &pose, Array<Slot *> &setupPose) {
	for (size_t i = 0, found = 0, done = _slots.size();; ++i) {
		if (_inFolder[pose[i]->getData().getIndex()]) {
			pose[i] = setupPose[_slots[found]];
			if (++found == done) break;
		}
	}
}

void DrawOrderFolderTimeline::apply(Array<Slot *> &pose, Array<Slot *> &setupPose, Array<int> &drawOrderIndices) {
	for (size_t i = 0, found = 0, done = _slots.size();; ++i) {
		if (_inFolder[pose[i]->getData().getIndex()]) {
			pose[i] = setupPose[_slots[drawOrderIndices[found]]];
			if (++found == done) break;
		}
	}
}
