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

RTTI_IMPL(PathConstraintSpacingTimeline, ConstraintTimeline1)

PathConstraintSpacingTimeline::PathConstraintSpacingTimeline(size_t frameCount, size_t bezierCount, int constraintIndex)
	: ConstraintTimeline1(frameCount, bezierCount, constraintIndex, Property_PathConstraintSpacing) {
}

PathConstraintSpacingTimeline::~PathConstraintSpacingTimeline() {
}

void PathConstraintSpacingTimeline::apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add,
										  bool out, bool appliedPose) {
	SP_UNUSED(lastTime);
	SP_UNUSED(events);
	SP_UNUSED(out);

	PathConstraint *constraint = (PathConstraint *) skeleton._constraints[_constraintIndex];
	if (constraint->isActive()) {
		PathConstraintPose &pose = appliedPose ? *constraint->_appliedPose : constraint->_pose;
		PathConstraintData &data = constraint->_data;
		pose._spacing = getAbsoluteValue(time, alpha, from, false, pose._spacing, data._setupPose._spacing);
	}
}
