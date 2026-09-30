#ifndef Spine_PhysicsConstraintPose_h
#define Spine_PhysicsConstraintPose_h

#include <spine/Pose.h>
#include <spine/RTTI.h>

namespace spine {

	class SP_API PhysicsConstraintPose : public Pose<PhysicsConstraintPose> {
		friend class PhysicsConstraint;
		friend class PhysicsConstraintTimeline;
		friend class SkeletonJson;
		friend class SkeletonBinary;

	private:
		float _inertia;
		float _strength;
		float _damping;
		float _massInverse;
		float _wind;
		float _gravity;
		float _mix;

	public:
		PhysicsConstraintPose();
		virtual ~PhysicsConstraintPose();

		virtual void set(PhysicsConstraintPose &pose) override;

		float getInertia();
		void setInertia(float inertia);

		float getStrength();
		void setStrength(float strength);

		float getDamping();
		void setDamping(float damping);

		float getMassInverse();
		void setMassInverse(float massInverse);

		float getWind();
		void setWind(float wind);

		float getGravity();
		void setGravity(float gravity);

		float getMix();
		void setMix(float mix);
	};
}

#endif
