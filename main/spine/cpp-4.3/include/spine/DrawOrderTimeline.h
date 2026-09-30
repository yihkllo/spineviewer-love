#ifndef Spine_DrawOrderTimeline_h
#define Spine_DrawOrderTimeline_h

#include <spine/Timeline.h>

namespace spine {

	class SP_API DrawOrderTimeline : public Timeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		static PropertyId getPropertyId();

		explicit DrawOrderTimeline(size_t frameCount);

		virtual void apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
						   bool appliedPose) override;

		size_t getFrameCount();

		Array<Array<int>> &getDrawOrders();

		void setFrame(size_t frame, float time, Array<int> *drawOrder);

	private:
		Array<Array<int>> _drawOrders;
	};
}

#endif
