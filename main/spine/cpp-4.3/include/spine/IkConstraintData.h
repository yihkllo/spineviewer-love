#ifndef Spine_IkConstraintData_h
#define Spine_IkConstraintData_h

#include <spine/Array.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>
#include <spine/ConstraintData.h>
#include <spine/PosedData.h>
#include <spine/IkConstraintPose.h>

namespace spine {
	class BoneData;
	class IkConstraint;

	class SP_API IkConstraintData : public ConstraintDataGeneric<IkConstraint, IkConstraintPose> {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		friend class IkConstraint;

		friend class Skeleton;

		friend class IkConstraintTimeline;

		RTTI_DECL

	public:
		explicit IkConstraintData(const String &name);

		virtual Constraint &create(Skeleton &skeleton) override;

		Array<BoneData *> &getBones();

		BoneData &getTarget();

		void setTarget(BoneData &inValue);

		ScaleYMode getScaleYMode();

		void setScaleYMode(ScaleYMode scaleYMode);

	private:
		Array<BoneData *> _bones;
		BoneData *_target;
		ScaleYMode _scaleYMode;
	};
}

#endif
