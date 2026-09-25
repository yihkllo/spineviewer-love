
#include <spine/PathConstraintPositionTimeline.h>

#include <spine/Event.h>
#include <spine/Skeleton.h>

#include <spine/Animation.h>
#include <spine/PathConstraint.h>
#include <spine/PathConstraintData.h>
#include <spine/Property.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>

using namespace spine;

RTTI_IMPL(PathConstraintPositionTimeline, CurveTimeline1)

PathConstraintPositionTimeline::PathConstraintPositionTimeline(size_t frameCount, size_t bezierCount,
															   int pathConstraintIndex) : CurveTimeline1(frameCount,
																										 bezierCount),
																						  _constraintIndex(
																								  pathConstraintIndex) {
	PropertyId ids[] = {((PropertyId) Property_PathConstraintPosition << 32) | pathConstraintIndex};
	setPropertyIds(ids, 1);
}

PathConstraintPositionTimeline::~PathConstraintPositionTimeline() {
}

void PathConstraintPositionTimeline::apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents,
										   float alpha, MixBlend blend, MixDirection direction) {
	SP_UNUSED(lastTime);
	SP_UNUSED(pEvents);
	SP_UNUSED(direction);

	PathConstraint *constraint = skeleton._pathConstraints[_constraintIndex];
	if (constraint->_active) constraint->_position = getAbsoluteValue(time, alpha, blend, constraint->_position, constraint->_data._position);
}
