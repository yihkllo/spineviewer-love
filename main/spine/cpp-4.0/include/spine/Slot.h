
#ifndef Spine_Slot_h
#define Spine_Slot_h

#include <spine/Vector.h>
#include <spine/SpineObject.h>
#include <spine/Color.h>

namespace spine {
	class SlotData;

	class Bone;

	class Skeleton;

	class Attachment;

	class SP_API Slot : public SpineObject {
		friend class VertexAttachment;

		friend class Skeleton;

		friend class SkeletonBounds;

		friend class SkeletonClipping;

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

	public:
		Slot(SlotData &data, Bone &bone);

		void setToSetupPose();

		SlotData &getData();

		Bone &getBone();

		Skeleton &getSkeleton();

		Color &getColor();

		Color &getDarkColor();

		bool hasDarkColor();

		Attachment *getAttachment();

		void setAttachment(Attachment *inValue);

		int getAttachmentState();

		void setAttachmentState(int state);

		float getAttachmentTime();

		void setAttachmentTime(float inValue);

		Vector<float> &getDeform();

	private:
		SlotData &_data;
		Bone &_bone;
		Skeleton &_skeleton;
		Color _color;
		Color _darkColor;
		bool _hasDarkColor;
		Attachment *_attachment;
		int _attachmentState;
		float _attachmentTime;
		Vector<float> _deform;
	};
}

#endif
