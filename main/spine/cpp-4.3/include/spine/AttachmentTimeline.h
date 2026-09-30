#ifndef Spine_AttachmentTimeline_h
#define Spine_AttachmentTimeline_h

#include <spine/Timeline.h>
#include <spine/SpineObject.h>
#include <spine/Array.h>
#include <spine/SpineString.h>
#include <spine/SlotTimeline.h>

namespace spine {

	class Skeleton;

	class Slot;

	class SlotPose;

	class Event;

	class SP_API AttachmentTimeline : public Timeline, public SlotTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit AttachmentTimeline(size_t frameCount, int slotIndex);

		virtual ~AttachmentTimeline();

		virtual void apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
						   bool appliedPose) override;

		void setFrame(int frame, float time, const String &attachmentName);

		Array<String> &getAttachmentNames();

		virtual int getSlotIndex() override {
			return _slotIndex;
		}

		virtual void setSlotIndex(int inValue) override {
			_slotIndex = inValue;
		}

	protected:
		int _slotIndex;
		Array<String> _attachmentNames;

		void setAttachment(Skeleton &skeleton, SlotPose &pose, String *attachmentName);
	};
}

#endif
