
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
		explicit AttachmentTimeline(int frameCount);

		virtual void apply(Skeleton& skeleton, float lastTime, float time, Vector<Event*>* pEvents, float alpha, MixBlend blend, MixDirection direction);

		virtual int getPropertyId();

		void setFrame(int frameIndex, float time, const String& attachmentName);

		size_t getSlotIndex();
		void setSlotIndex(size_t inValue);
		Vector<float>& getFrames();
		Vector<String>& getAttachmentNames();
		size_t getFrameCount();
	private:
		size_t _slotIndex;
		Vector<float> _frames;
		Vector<String> _attachmentNames;

        void setAttachment(Skeleton& skeleton, Slot& slot, String* attachmentName);
    };
}

#endif
