
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/PathConstraintSpacingTimeline.h>

#include <spine/Skeleton.h>
#include <spine/Event.h>

#include <spine/Animation.h>
#include <spine/TimelineType.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>
#include <spine/PathConstraint.h>
#include <spine/PathConstraintData.h>

using namespace spine;

RTTI_IMPL(PathConstraintSpacingTimeline, PathConstraintPositionTimeline)

PathConstraintSpacingTimeline::PathConstraintSpacingTimeline(int frameCount) : PathConstraintPositionTimeline(
		frameCount) {
}

void PathConstraintSpacingTimeline::apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents,
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
			constraint._spacing = constraint._data._spacing;
			return;
		case MixBlend_First:
			constraint._spacing += (constraint._data._spacing - constraint._spacing) * alpha;
			return;
		default:
			return;
		}
	}

	float spacing;
	if (time >= _frames[_frames.size() - ENTRIES]) {
		spacing = _frames[_frames.size() + PREV_VALUE];
	} else {
		int frame = Animation::binarySearch(_frames, time, ENTRIES);
		spacing = _frames[frame + PREV_VALUE];
		float frameTime = _frames[frame];
		float percent = getCurvePercent(frame / ENTRIES - 1,
			1 - (time - frameTime) / (_frames[frame + PREV_TIME] - frameTime));

		spacing += (_frames[frame + VALUE] - spacing) * percent;
	}

	if (blend == MixBlend_Setup)
		constraint._spacing = constraint._data._spacing + (spacing - constraint._data._spacing) * alpha;
	else
		constraint._spacing += (spacing - constraint._spacing) * alpha;
}

int PathConstraintSpacingTimeline::getPropertyId() {
	return ((int) TimelineType_PathConstraintSpacing << 24) + _pathConstraintIndex;
}
