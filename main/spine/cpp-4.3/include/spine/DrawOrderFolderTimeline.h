#ifndef Spine_DrawOrderFolderTimeline_h
#define Spine_DrawOrderFolderTimeline_h

#include <spine/Timeline.h>

namespace spine {
	class Slot;

	class SP_API DrawOrderFolderTimeline : public Timeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		DrawOrderFolderTimeline(size_t frameCount, Array<int> &slots, size_t slotCount);

		virtual void apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
						   bool appliedPose) override;

		size_t getFrameCount();

		Array<int> &getSlots();

		Array<Array<int>> &getDrawOrders();

		void setFrame(size_t frame, float time, Array<int> *drawOrder);

	private:
		Array<int> _slots;
		Array<bool> _inFolder;
		Array<Array<int>> _drawOrders;

		void setup(Array<Slot *> &pose, Array<Slot *> &setupPose);
		void apply(Array<Slot *> &pose, Array<Slot *> &setupPose, Array<int> &drawOrder);
	};
}

#endif
