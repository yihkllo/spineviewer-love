
#include <spine/DrawOrderTimeline.h>

#include <spine/Event.h>
#include <spine/Skeleton.h>

#include <spine/Animation.h>
#include <spine/Property.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>

using namespace spine;

RTTI_IMPL(DrawOrderTimeline, Timeline)

DrawOrderTimeline::DrawOrderTimeline(size_t frameCount) : Timeline(frameCount, 1) {
	PropertyId ids[] = {((PropertyId) Property_DrawOrder << 32)};
	setPropertyIds(ids, 1);

	_drawOrders.ensureCapacity(frameCount);
	for (size_t i = 0; i < frameCount; ++i) {
		Vector<int> vec;
		_drawOrders.add(vec);
	}
}

void DrawOrderTimeline::apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha,
							  MixBlend blend, MixDirection direction) {
	SP_UNUSED(lastTime);
	SP_UNUSED(pEvents);
	SP_UNUSED(alpha);

	Vector<Slot *> &drawOrder = skeleton._drawOrder;
	Vector<Slot *> &slots = skeleton._slots;
	if (direction == MixDirection_Out) {
		if (blend == MixBlend_Setup) {
			drawOrder.clear();
			drawOrder.ensureCapacity(slots.size());
			for (size_t i = 0, n = slots.size(); i < n; ++i)
				drawOrder.add(slots[i]);
		}
		return;
	}

	if (time < _frames[0]) {
		if (blend == MixBlend_Setup || blend == MixBlend_First) {
			drawOrder.clear();
			drawOrder.ensureCapacity(slots.size());
			for (size_t i = 0, n = slots.size(); i < n; ++i)
				drawOrder.add(slots[i]);
		}
		return;
	}

	Vector<int> &drawOrderToSetupIndex = _drawOrders[Animation::search(_frames, time)];
	if (drawOrderToSetupIndex.size() == 0) {
		drawOrder.clear();
		for (size_t i = 0, n = slots.size(); i < n; ++i)
			drawOrder.add(slots[i]);
	} else {
		for (size_t i = 0, n = drawOrderToSetupIndex.size(); i < n; ++i)
			drawOrder[i] = slots[drawOrderToSetupIndex[i]];
	}
}

void DrawOrderTimeline::setFrame(size_t frame, float time, Vector<int> &drawOrder) {
	_frames[frame] = time;
	_drawOrders[frame].clear();
	_drawOrders[frame].addAll(drawOrder);
}

Vector<Vector<int>> &DrawOrderTimeline::getDrawOrders() {
	return _drawOrders;
}
