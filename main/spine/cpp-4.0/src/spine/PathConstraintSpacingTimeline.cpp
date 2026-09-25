
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/PathConstraintSpacingTimeline.h>

#include <spine/Event.h>
#include <spine/Skeleton.h>

#include <spine/Animation.h>
#include <spine/PathConstraint.h>
#include <spine/PathConstraintData.h>
#include <spine/Property.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>

using namespace spine;

RTTI_IMPL(PathConstraintSpacingTimeline, PathConstraintPositionTimeline)

PathConstraintSpacingTimeline::PathConstraintSpacingTimeline(size_t frameCount, size_t bezierCount,
															 int pathConstraintIndex) : CurveTimeline1(frameCount,
																									   bezierCount),
																						_pathConstraintIndex(
																								pathConstraintIndex) {
	PropertyId ids[] = {((PropertyId) Property_PathConstraintSpacing << 32) | pathConstraintIndex};
	setPropertyIds(ids, 1);
}

void PathConstraintSpacingTimeline::apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents,
										  float alpha, MixBlend blend, MixDirection direction) {
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

	float spacing = getCurveValue(time);

	if (blend == MixBlend_Setup)
		constraint._spacing = constraint._data._spacing + (spacing - constraint._data._spacing) * alpha;
	else
		constraint._spacing += (spacing - constraint._spacing) * alpha;
}
