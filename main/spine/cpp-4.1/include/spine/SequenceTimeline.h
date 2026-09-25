
#ifndef Spine_SequenceTimeline_h
#define Spine_SequenceTimeline_h

#include <spine/Timeline.h>
#include <spine/Sequence.h>

namespace spine {
	class Attachment;

	class SP_API SequenceTimeline : public Timeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit SequenceTimeline(size_t frameCount, int slotIndex, spine::Attachment *attachment);

		virtual ~SequenceTimeline();

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction);

		void setFrame(int frame, float time, SequenceMode mode, int index, float delay);

		int getSlotIndex() { return _slotIndex; };

		void setSlotIndex(int inValue) { _slotIndex = inValue; }

		Attachment *getAttachment() { return _attachment; }

	protected:
		int _slotIndex;
		Attachment *_attachment;

		static const int ENTRIES = 3;
		static const int MODE = 1;
		static const int DELAY = 2;
	};
}

#endif
