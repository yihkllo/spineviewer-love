
#ifndef Spine_AttachmentTimeline_h
#define Spine_AttachmentTimeline_h

#include <spine/Timeline.h>
#include <spine/SpineObject.h>
#include <spine/Vector.h>
#include <spine/MixBlend.h>
#include <spine/MixDirection.h>
#include <spine/SpineString.h>

namespace spine {

	class Skeleton;

	class Slot;

	class Event;

	class SP_API AttachmentTimeline : public Timeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit AttachmentTimeline(size_t frameCount, int slotIndex);

		virtual ~AttachmentTimeline();

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction);

		void setFrame(int frame, float time, const String &attachmentName);

		Vector<String> &getAttachmentNames();

		int getSlotIndex() { return _slotIndex; }

		void setSlotIndex(int inValue) { _slotIndex = inValue; }

	protected:
		int _slotIndex;

		Vector<String> _attachmentNames;

		void setAttachment(Skeleton &skeleton, Slot &slot, String *attachmentName);
	};
}

#endif
