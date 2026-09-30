#include <spine/PhysicsConstraintTimeline.h>

#include <spine/Event.h>
#include <spine/Skeleton.h>

#include <spine/Animation.h>
#include <spine/Property.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>
#include <spine/PhysicsConstraint.h>
#include <cfloat>

using namespace spine;

RTTI_IMPL_MULTI(PhysicsConstraintTimeline, CurveTimeline, ConstraintTimeline)
RTTI_IMPL(PhysicsConstraintInertiaTimeline, PhysicsConstraintTimeline)
RTTI_IMPL(PhysicsConstraintStrengthTimeline, PhysicsConstraintTimeline)
RTTI_IMPL(PhysicsConstraintDampingTimeline, PhysicsConstraintTimeline)
RTTI_IMPL(PhysicsConstraintMassTimeline, PhysicsConstraintTimeline)
RTTI_IMPL(PhysicsConstraintWindTimeline, PhysicsConstraintTimeline)
RTTI_IMPL(PhysicsConstraintGravityTimeline, PhysicsConstraintTimeline)
RTTI_IMPL(PhysicsConstraintMixTimeline, PhysicsConstraintTimeline)
RTTI_IMPL_MULTI(PhysicsConstraintResetTimeline, Timeline, ConstraintTimeline)

PhysicsConstraintTimeline::PhysicsConstraintTimeline(size_t frameCount, size_t bezierCount, int constraintIndex, Property property)
	: CurveTimeline1(frameCount, bezierCount), ConstraintTimeline(), _constraintIndex(constraintIndex) {
	PropertyId ids[] = {((PropertyId) property << 32) | constraintIndex};
	setPropertyIds(ids, 1);
}

void PhysicsConstraintTimeline::apply(Skeleton &skeleton, float, float time, Array<Event *> *, float alpha, MixFrom from, bool add, bool out,
									  bool appliedPose) {
	SP_UNUSED(out);
	if (add && !_additive) add = false;
	if (_constraintIndex == -1) {
		float value = time >= _frames[0] ? getCurveValue(time) : 0;

		Array<PhysicsConstraint *> &physicsConstraints = skeleton.getPhysicsConstraints();
		for (size_t i = 0; i < physicsConstraints.size(); i++) {
			PhysicsConstraint *constraint = physicsConstraints[i];
			if (constraint->isActive() && global(constraint->_data)) {
				PhysicsConstraintPose &pose = appliedPose ? *constraint->_appliedPose : constraint->_pose;
				set(pose, getAbsoluteValue(time, alpha, from, add, get(pose), get(constraint->_data._setupPose), value));
			}
		}
	} else {
		PhysicsConstraint *constraint = static_cast<PhysicsConstraint *>(skeleton.getConstraints()[_constraintIndex]);
		if (constraint->isActive()) {
			PhysicsConstraintPose &pose = appliedPose ? *constraint->_appliedPose : constraint->_pose;
			set(pose, getAbsoluteValue(time, alpha, from, add, get(pose), get(constraint->_data._setupPose)));
		}
	}
}

void PhysicsConstraintResetTimeline::apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *, float alpha, MixFrom from, bool add,
										   bool out, bool appliedPose) {
	PhysicsConstraint *constraint = nullptr;
	if (_constraintIndex != -1) {
		constraint = static_cast<PhysicsConstraint *>(skeleton.getConstraints()[_constraintIndex]);
		if (!constraint->isActive()) return;
	}

	if (lastTime > time) {
		apply(skeleton, lastTime, FLT_MAX, nullptr, alpha, MixFrom_Current, false, false, false);
		lastTime = -1;
	} else if (lastTime >= _frames[_frames.size() - 1])
		return;
	if (time < _frames[0]) return;

	if (lastTime < _frames[0] || time >= _frames[Animation::search(_frames, lastTime) + 1]) {
		if (constraint != nullptr)
			constraint->reset(skeleton);
		else {
			Array<PhysicsConstraint *> &physicsConstraints = skeleton.getPhysicsConstraints();
			for (size_t i = 0; i < physicsConstraints.size(); i++) {
				constraint = physicsConstraints[i];
				if (constraint->isActive()) constraint->reset(skeleton);
			}
		}
	}
}
