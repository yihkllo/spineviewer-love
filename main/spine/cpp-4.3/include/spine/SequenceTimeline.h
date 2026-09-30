#ifndef Spine_SequenceTimeline_h
#define Spine_SequenceTimeline_h

#include <spine/Timeline.h>
#include <spine/SlotTimeline.h>
#include <spine/Sequence.h>

namespace spine {
	class Attachment;
	class HasTextureRegion;
	class Slot;

	class SP_API SequenceTimeline : public Timeline, public SlotTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit SequenceTimeline(size_t frameCount, int slotIndex, Attachment &attachment);

		virtual ~SequenceTimeline();

		virtual void apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
						   bool appliedPose) override;

		void setFrame(int frame, float time, SequenceMode mode, int index, float delay);

		Attachment &getAttachment() {
			return *(Attachment *) _attachment;
		}

		virtual int getSlotIndex() override;

		virtual void setSlotIndex(int inValue) override;

	protected:
		int _slotIndex;
		Attachment *_attachment;

		void setupPose(Slot &slot, bool appliedPose);
		void applyToSlot(Slot &slot, bool appliedPose, Sequence &sequence, float time, float before, int modeAndIndex, float delay);

		static const int ENTRIES = 3;
		static const int MODE = 1;
		static const int DELAY = 2;
	};
}

#endif
