#ifndef Spine_IkConstraint_h
#define Spine_IkConstraint_h

#include <spine/Constraint.h>
#include <spine/ConstraintData.h>
#include <spine/IkConstraintData.h>
#include <spine/IkConstraintPose.h>
#include <spine/Array.h>

namespace spine {
	class Skeleton;
	class Bone;
	class BonePose;

	class IkConstraintBase : public ConstraintGeneric<IkConstraint, IkConstraintData, IkConstraintPose> {
	public:
		IkConstraintBase(IkConstraintData &data) : ConstraintGeneric<IkConstraint, IkConstraintData, IkConstraintPose>(data) {
		}
	};

	class SP_API IkConstraint : public IkConstraintBase {
		friend class Skeleton;

		friend class IkConstraintTimeline;

		RTTI_DECL

	public:
		IkConstraint(IkConstraintData &data, Skeleton &skeleton);

		virtual IkConstraint &copy(Skeleton &skeleton);

		virtual void update(Skeleton &skeleton, Physics physics) override;

		virtual void sort(Skeleton &skeleton) override;

		virtual bool isSourceActive() override;

		Array<BonePose *> &getBones();

		Bone &getTarget();

		void setTarget(Bone &inValue);

		static void apply(Skeleton &skeleton, BonePose &bone, float targetX, float targetY, bool compress, bool stretch, ScaleYMode scaleYMode,
						  float mix);

		static void apply(Skeleton &skeleton, BonePose &parent, BonePose &child, float targetX, float targetY, int bendDirection, bool stretch,
						  ScaleYMode scaleYMode, float softness, float mix);

	private:
		Array<BonePose *> _bones;
		Bone *_target;
	};
}

#endif
