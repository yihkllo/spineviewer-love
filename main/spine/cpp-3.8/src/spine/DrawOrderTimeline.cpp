
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/DrawOrderTimeline.h>

#include <spine/Skeleton.h>
#include <spine/Event.h>

#include <spine/Animation.h>
#include <spine/TimelineType.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>

using namespace spine;

RTTI_IMPL(DrawOrderTimeline, Timeline)

DrawOrderTimeline::DrawOrderTimeline(int frameCount) : Timeline() {
	_frames.ensureCapacity(frameCount);
	_drawOrders.ensureCapacity(frameCount);

	_frames.setSize(frameCount, 0);

	for (int i = 0; i < frameCount; ++i) {
		Vector<int> vec;
		_drawOrders.add(vec);
	}
}

void DrawOrderTimeline::apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha,
	MixBlend blend, MixDirection direction
) {
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

	size_t frame;
	if (time >= _frames[_frames.size() - 1]) {
		frame = _frames.size() - 1;
	} else
		frame = (size_t)Animation::binarySearch(_frames, time) - 1;

	Vector<int> &drawOrderToSetupIndex = _drawOrders[frame];
	if (drawOrderToSetupIndex.size() == 0) {
		drawOrder.clear();
		for (size_t i = 0, n = slots.size(); i < n; ++i)
			drawOrder.add(slots[i]);
	} else {
		for (size_t i = 0, n = drawOrderToSetupIndex.size(); i < n; ++i)
			drawOrder[i] = slots[drawOrderToSetupIndex[i]];
	}
}

int DrawOrderTimeline::getPropertyId() {
	return ((int) TimelineType_DrawOrder << 24);
}

void DrawOrderTimeline::setFrame(size_t frameIndex, float time, Vector<int> &drawOrder) {
	_frames[frameIndex] = time;
	_drawOrders[frameIndex].clear();
	_drawOrders[frameIndex].addAll(drawOrder);
}

Vector<float> &DrawOrderTimeline::getFrames() {
	return _frames;
}

Vector<Vector<int> > &DrawOrderTimeline::getDrawOrders() {
	return _drawOrders;
}

size_t DrawOrderTimeline::getFrameCount() {
	return _frames.size();
}
