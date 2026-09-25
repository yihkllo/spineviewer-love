
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

	PathConstraint *constraint = skeleton._pathConstraints[_pathConstraintIndex];
	if (constraint->_active)
		constraint->_spacing = getAbsoluteValue(time, alpha, blend, constraint->_spacing, constraint->_data._spacing);
}
