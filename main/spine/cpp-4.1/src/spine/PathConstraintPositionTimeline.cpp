
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
																						  _pathConstraintIndex(
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

	float position = getCurveValue(time);

	if (blend == MixBlend_Setup)
		constraint._position = constraint._data._position + (position - constraint._data._position) * alpha;
	else
		constraint._position += (position - constraint._position) * alpha;
}
