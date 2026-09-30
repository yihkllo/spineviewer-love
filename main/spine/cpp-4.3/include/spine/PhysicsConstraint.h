#ifndef Spine_PhysicsConstraint_h
#define Spine_PhysicsConstraint_h

#include <spine/Constraint.h>
#include <spine/PhysicsConstraintData.h>
#include <spine/PhysicsConstraintPose.h>
#include <spine/BonePose.h>
#include <spine/Array.h>

namespace spine {
	class Skeleton;
	class BonePose;
	class PhysicsConstraintPose;

	class PhysicsConstraintBase : public ConstraintGeneric<PhysicsConstraint, PhysicsConstraintData, PhysicsConstraintPose> {
	public:
		PhysicsConstraintBase(PhysicsConstraintData &data)
			: ConstraintGeneric<PhysicsConstraint, PhysicsConstraintData, PhysicsConstraintPose>(data) {
		}
	};

	class SP_API PhysicsConstraint : public PhysicsConstraintBase {
		friend class Skeleton;
		friend class PhysicsConstraintTimeline;
		friend class PhysicsConstraintInertiaTimeline;
		friend class PhysicsConstraintStrengthTimeline;
		friend class PhysicsConstraintDampingTimeline;
		friend class PhysicsConstraintMassTimeline;
		friend class PhysicsConstraintWindTimeline;
		friend class PhysicsConstraintGravityTimeline;
		friend class PhysicsConstraintMixTimeline;
		friend class PhysicsConstraintResetTimeline;

	public:
		RTTI_DECL

		PhysicsConstraint(PhysicsConstraintData &data, Skeleton &skeleton);

		void update(Skeleton &skeleton, Physics physics) override;
		void sort(Skeleton &skeleton) override;
		bool isSourceActive() override;
		PhysicsConstraint &copy(Skeleton &skeleton);

		void reset(Skeleton &skeleton);

		void translate(float x, float y);

		void rotate(float x, float y, float degrees);

		BonePose &getBone();
		void setBone(BonePose &bone);

	private:
		BonePose *_bone;

		bool _reset;
		float _ux, _uy, _cx, _cy, _tx, _ty;
		float _xOffset, _xLag, _xVelocity;
		float _yOffset, _yLag, _yVelocity;
		float _rotateOffset, _rotateLag, _rotateVelocity;
		float _scaleOffset, _scaleLag, _scaleVelocity;
		float _remaining, _lastTime;
	};
}

#endif
