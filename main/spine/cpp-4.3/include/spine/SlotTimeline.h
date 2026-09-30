#ifndef Spine_SlotTimeline_h
#define Spine_SlotTimeline_h

#include <spine/dll.h>
#include <spine/RTTI.h>

namespace spine {

	class SP_API SlotTimeline {
		RTTI_DECL_NOPARENT

		friend class AlphaTimeline;
		friend class AttachmentTimeline;
		friend class SequenceTimeline;
		friend class SlotCurveTimeline;

	public:
		SlotTimeline();
		virtual ~SlotTimeline();

		virtual int getSlotIndex() = 0;

		virtual void setSlotIndex(int inValue) = 0;
	};
}

#endif
