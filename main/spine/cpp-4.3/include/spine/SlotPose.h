#ifndef SPINE_SLOTPOSE_H_
#define SPINE_SLOTPOSE_H_

#include <spine/SpineObject.h>
#include <spine/RTTI.h>
#include <spine/Pose.h>
#include <spine/Color.h>
#include <spine/Array.h>

namespace spine {
	class Attachment;
	class VertexAttachment;

	class SP_API SlotPose : public Pose<SlotPose> {
		friend class Slot;
		friend class SlotCurveTimeline;
		friend class DeformTimeline;
		friend class RGBATimeline;
		friend class RGBTimeline;
		friend class AlphaTimeline;
		friend class RGBA2Timeline;
		friend class RGB2Timeline;
		friend class PathConstraint;
		friend class SkeletonJson;
		friend class SkeletonBinary;
		friend class AnimationState;

	protected:
		Color _color;
		Color _darkColor;
		bool _hasDarkColor;
		Attachment *_attachment;
		int _sequenceIndex;
		Array<float> _deform;

	public:
		SlotPose();
		virtual ~SlotPose();

		virtual void set(SlotPose &pose) override;

		Color &getColor();

		Color &getDarkColor();

		bool hasDarkColor();

		void setHasDarkColor(bool hasDarkColor);

		Attachment *getAttachment();

		void setAttachment(Attachment *attachment);

		int getSequenceIndex();
		void setSequenceIndex(int sequenceIndex);

		Array<float> &getDeform();
	};
}

#endif
