
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

RTTI_IMPL(PhysicsConstraintTimeline, CurveTimeline)
RTTI_IMPL(PhysicsConstraintInertiaTimeline, PhysicsConstraintTimeline)
RTTI_IMPL(PhysicsConstraintStrengthTimeline, PhysicsConstraintTimeline)
RTTI_IMPL(PhysicsConstraintDampingTimeline, PhysicsConstraintTimeline)
RTTI_IMPL(PhysicsConstraintMassTimeline, PhysicsConstraintTimeline)
RTTI_IMPL(PhysicsConstraintWindTimeline, PhysicsConstraintTimeline)
RTTI_IMPL(PhysicsConstraintGravityTimeline, PhysicsConstraintTimeline)
RTTI_IMPL(PhysicsConstraintMixTimeline, PhysicsConstraintTimeline)
RTTI_IMPL(PhysicsConstraintResetTimeline, Timeline)

PhysicsConstraintTimeline::PhysicsConstraintTimeline(size_t frameCount, size_t bezierCount,
													 int constraintIndex, Property property) : CurveTimeline1(frameCount, bezierCount),
																							   _constraintIndex(constraintIndex) {
	PropertyId ids[] = {((PropertyId) property << 32) | constraintIndex};
	setPropertyIds(ids, 1);
}

void PhysicsConstraintTimeline::apply(Skeleton &skeleton, float, float time, Vector<Event *> *,
									  float alpha, MixBlend blend, MixDirection) {
	if (_constraintIndex == -1) {
		float value = time >= _frames[0] ? getCurveValue(time) : 0;

		Vector<PhysicsConstraint *> &physicsConstraints = skeleton.getPhysicsConstraints();
		for (size_t i = 0; i < physicsConstraints.size(); i++) {
			PhysicsConstraint *constraint = physicsConstraints[i];
			if (constraint->_active && global(constraint->_data))
				set(constraint, getAbsoluteValue(time, alpha, blend, get(constraint), setup(constraint), value));
		}
	} else {
		PhysicsConstraint *constraint = skeleton.getPhysicsConstraints()[_constraintIndex];
		if (constraint->_active) set(constraint, getAbsoluteValue(time, alpha, blend, get(constraint), setup(constraint)));
	}
}

void PhysicsConstraintResetTimeline::apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *, float alpha, MixBlend blend, MixDirection direction) {
	PhysicsConstraint *constraint = nullptr;
	if (_constraintIndex != -1) {
		constraint = skeleton.getPhysicsConstraints()[_constraintIndex];
		if (!constraint->_active) return;
	}

	if (lastTime > time) {
		apply(skeleton, lastTime, FLT_MAX, nullptr, alpha, blend, direction);
		lastTime = -1;
	} else if (lastTime >= _frames[_frames.size() - 1])
		return;
	if (time < _frames[0]) return;

	if (lastTime < _frames[0] || time >= _frames[Animation::search(_frames, lastTime) + 1]) {
		if (constraint != nullptr)
			constraint->reset();
		else {
			Vector<PhysicsConstraint *> &physicsConstraints = skeleton.getPhysicsConstraints();
			for (size_t i = 0; i < physicsConstraints.size(); i++) {
				constraint = physicsConstraints[i];
				if (constraint->_active) constraint->reset();
			}
		}
	}
}
