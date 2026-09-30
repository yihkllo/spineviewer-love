#ifndef Spine_SlotCurveTimeline_h
#define Spine_SlotCurveTimeline_h

#include <spine/CurveTimeline.h>
#include <spine/SlotTimeline.h>

namespace spine {
	class Slot;
	class SlotPose;

	class SP_API SlotCurveTimeline : public CurveTimeline, public SlotTimeline {
		friend class SkeletonBinary;
		friend class SkeletonJson;

		RTTI_DECL

	public:
		SlotCurveTimeline(size_t frameCount, size_t frameEntries, size_t bezierCount, int slotIndex);

		virtual ~SlotCurveTimeline();

		virtual void apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
						   bool appliedPose) override;

		virtual int getSlotIndex() override;

		virtual void setSlotIndex(int inValue) override;

	protected:

		virtual void _apply(Slot &slot, SlotPose &pose, float time, float alpha, MixFrom from, bool add) = 0;

		int _slotIndex;
	};
}

#endif
