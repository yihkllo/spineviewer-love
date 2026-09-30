#ifndef Spine_Slot_h
#define Spine_Slot_h

#include <spine/Posed.h>
#include <spine/SlotData.h>
#include <spine/SlotPose.h>
#include <spine/Array.h>
#include <spine/Color.h>
#include <spine/Update.h>

namespace spine {
	class Bone;
	class Skeleton;
	class Attachment;

	class SP_API Slot : public PosedGeneric<SlotData, SlotPose, SlotPose> {
		friend class VertexAttachment;

		friend class Skeleton;

		friend class SkeletonBounds;

		friend class SkeletonClipping;

		friend class SlotCurveTimeline;

		friend class AttachmentTimeline;

		friend class RGBATimeline;

		friend class RGBTimeline;

		friend class AlphaTimeline;

		friend class RGBA2Timeline;

		friend class RGB2Timeline;

		friend class DeformTimeline;

		friend class DrawOrderTimeline;

		friend class EventTimeline;

		friend class IkConstraintTimeline;

		friend class PathConstraintMixTimeline;

		friend class PathConstraintPositionTimeline;

		friend class PathConstraintSpacingTimeline;

		friend class ScaleTimeline;

		friend class ShearTimeline;

		friend class TransformConstraintTimeline;

		friend class TranslateTimeline;

		friend class TwoColorTimeline;

		friend class AnimationState;

	public:
		Slot(SlotData &data, Skeleton &skeleton);

		Bone &getBone();

		void setupPose() override;

	private:
		Skeleton &_skeleton;
		Bone &_bone;
		int _attachmentState;
	};
}

#endif
