
#ifndef Spine_DrawOrderTimeline_h
#define Spine_DrawOrderTimeline_h

#include <spine/Timeline.h>

namespace spine {
	class SP_API DrawOrderTimeline : public Timeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit DrawOrderTimeline(size_t frameCount);

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction);

		void setFrame(size_t frame, float time, Vector<int> &drawOrder);

		Vector <Vector<int>> &getDrawOrders();

	private:
		Vector <Vector<int>> _drawOrders;
	};
}

#endif
