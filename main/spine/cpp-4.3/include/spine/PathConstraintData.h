#ifndef Spine_PathConstraintData_h
#define Spine_PathConstraintData_h

#include <spine/ConstraintData.h>
#include <spine/PosedData.h>
#include <spine/Array.h>
#include <spine/PathConstraintPose.h>
#include <spine/dll.h>
#include <spine/PositionMode.h>
#include <spine/SpacingMode.h>
#include <spine/RotateMode.h>

namespace spine {
	class BoneData;
	class SlotData;
	class PathConstraint;
	class Skeleton;

	class SP_API PathConstraintData : public ConstraintDataGeneric<PathConstraint, PathConstraintPose> {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		friend class PathConstraint;

		friend class Skeleton;

		friend class PathConstraintMixTimeline;

		friend class PathConstraintPositionTimeline;

		friend class PathConstraintSpacingTimeline;

		RTTI_DECL
	public:
		explicit PathConstraintData(const String &name);

		virtual Constraint &create(Skeleton &skeleton) override;

		Array<BoneData *> &getBones();

		SlotData &getSlot();

		void setSlot(SlotData &slot);

		PositionMode getPositionMode();

		void setPositionMode(PositionMode positionMode);

		SpacingMode getSpacingMode();

		void setSpacingMode(SpacingMode spacingMode);

		RotateMode getRotateMode();

		void setRotateMode(RotateMode rotateMode);

		float getOffsetRotation();

		void setOffsetRotation(float offsetRotation);

	private:
		Array<BoneData *> _bones;
		SlotData *_slot;
		PositionMode _positionMode;
		SpacingMode _spacingMode;
		RotateMode _rotateMode;
		float _offsetRotation;
	};
}

#endif
