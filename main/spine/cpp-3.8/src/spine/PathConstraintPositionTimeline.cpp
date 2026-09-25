
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/PathConstraintPositionTimeline.h>

#include <spine/Skeleton.h>
#include <spine/Event.h>

#include <spine/Animation.h>
#include <spine/TimelineType.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>
#include <spine/PathConstraint.h>
#include <spine/PathConstraintData.h>

using namespace spine;

RTTI_IMPL(PathConstraintPositionTimeline, CurveTimeline)

const int PathConstraintPositionTimeline::ENTRIES = 2;
const int PathConstraintPositionTimeline::PREV_TIME = -2;
const int PathConstraintPositionTimeline::PREV_VALUE = -1;
const int PathConstraintPositionTimeline::VALUE = 1;

PathConstraintPositionTimeline::PathConstraintPositionTimeline(int frameCount) : CurveTimeline(frameCount),
	_pathConstraintIndex(0)
{
	_frames.setSize(frameCount * ENTRIES, 0);
}

PathConstraintPositionTimeline::~PathConstraintPositionTimeline() {
}

void PathConstraintPositionTimeline::apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents,
	float alpha, MixBlend blend, MixDirection direction
) {
	SP_UNUSED(lastTime);
	SP_UNUSED(pEvents);
	SP_UNUSED(direction);

	PathConstraint *constraintP = skeleton._pathConstraints[_pathConstraintIndex];
	PathConstraint &constraint = *constraintP;
	if (!constraint.isActive()) return;

	if (time < _frames[0]) {
		switch (blend) {
		case MixBlend_Setup:
			constraint._position = constraint._data._position;
			return;
		case MixBlend_First:
			constraint._position += (constraint._data._position - constraint._position) * alpha;
			return;
		default:
			return;
		}
	}

	float position;
	if (time >= _frames[_frames.size() - ENTRIES]) {
		position = _frames[_frames.size() + PREV_VALUE];
	} else {
		int frame = Animation::binarySearch(_frames, time, ENTRIES);
		position = _frames[frame + PREV_VALUE];
		float frameTime = _frames[frame];
		float percent = getCurvePercent(frame / ENTRIES - 1,
			1 - (time - frameTime) / (_frames[frame + PREV_TIME] - frameTime));

		position += (_frames[frame + VALUE] - position) * percent;
	}
	if (blend == MixBlend_Setup)
		constraint._position = constraint._data._position + (position - constraint._data._position) * alpha;
	else
		constraint._position += (position - constraint._position) * alpha;
}

int PathConstraintPositionTimeline::getPropertyId() {
	return ((int) TimelineType_PathConstraintPosition << 24) + _pathConstraintIndex;
}

void PathConstraintPositionTimeline::setFrame(int frameIndex, float time, float value) {
	frameIndex *= ENTRIES;
	_frames[frameIndex] = time;
	_frames[frameIndex + VALUE] = value;
}
