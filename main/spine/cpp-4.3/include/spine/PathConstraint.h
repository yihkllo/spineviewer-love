#ifndef Spine_PathConstraint_h
#define Spine_PathConstraint_h

#include <spine/Constraint.h>
#include <spine/ConstraintData.h>
#include <spine/PathConstraintData.h>
#include <spine/PathConstraintPose.h>
#include <spine/Array.h>

namespace spine {
	class Skeleton;
	class PathAttachment;
	class BonePose;
	class Slot;
	class Bone;
	class Skin;
	class Attachment;

	class PathConstraintBase : public ConstraintGeneric<PathConstraint, PathConstraintData, PathConstraintPose> {
	public:
		PathConstraintBase(PathConstraintData &data) : ConstraintGeneric<PathConstraint, PathConstraintData, PathConstraintPose>(data) {
		}
	};

	class SP_API PathConstraint : public PathConstraintBase {
		friend class Skeleton;
		friend class PathConstraintMixTimeline;
		friend class PathConstraintPositionTimeline;
		friend class PathConstraintPositionTimeline;
		friend class PathConstraintSpacingTimeline;

		RTTI_DECL

	public:
		static const float epsilon;
		static const int NONE;
		static const int BEFORE;
		static const int AFTER;

		PathConstraint(PathConstraintData &data, Skeleton &skeleton);

		PathConstraint &copy(Skeleton &skeleton);

		virtual void update(Skeleton &skeleton, Physics physics) override;

		virtual void sort(Skeleton &skeleton) override;

		virtual bool isSourceActive() override;

		Array<BonePose *> &getBones();

		Slot &getSlot();

		void setSlot(Slot &slot);

	private:
		Array<BonePose *> _bones;
		Slot *_slot;

		Array<float> _spaces;
		Array<float> _positions;
		Array<float> _world;
		Array<float> _curves;
		Array<float> _lengths;
		Array<float> _segments;

		Array<float> &computeWorldPositions(Skeleton &skeleton, PathAttachment &path, int spacesCount, bool tangents);

		void addBeforePosition(float p, Array<float> &temp, int i, Array<float> &output, int o);

		void addAfterPosition(float p, Array<float> &temp, int i, Array<float> &output, int o);

		void addCurvePosition(float p, float x1, float y1, float cx1, float cy1, float cx2, float cy2, float x2, float y2, Array<float> &output,
							  int o, bool tangents);

		void sortPathSlot(Skeleton &skeleton, Skin &skin, int slotIndex, Bone &slotBone);

		void sortPath(Skeleton &skeleton, Attachment *attachment, Bone &slotBone);
	};
}

#endif
