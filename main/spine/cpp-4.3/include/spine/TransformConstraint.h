#ifndef Spine_TransformConstraint_h
#define Spine_TransformConstraint_h

#include <spine/Constraint.h>
#include <spine/TransformConstraintData.h>
#include <spine/TransformConstraintPose.h>
#include <spine/Array.h>

namespace spine {
	class Skeleton;
	class Bone;
	class BonePose;

	class TransformConstraintBase : public ConstraintGeneric<TransformConstraint, TransformConstraintData, TransformConstraintPose> {
	public:
		TransformConstraintBase(TransformConstraintData &data)
			: ConstraintGeneric<TransformConstraint, TransformConstraintData, TransformConstraintPose>(data) {
		}
	};

	class SP_API TransformConstraint : public TransformConstraintBase {
		friend class Skeleton;
		friend class TransformConstraintTimeline;

	public:
		RTTI_DECL

		TransformConstraint(TransformConstraintData &data, Skeleton &skeleton);

		virtual TransformConstraint &copy(Skeleton &skeleton);

		void update(Skeleton &skeleton, Physics physics) override;

		void sort(Skeleton &skeleton) override;

		bool isSourceActive() override;

		Array<BonePose *> &getBones();

		Bone &getSource();

		void setSource(Bone &source);

	private:
		Array<BonePose *> _bones;
		Bone *_source;
	};
}

#endif
