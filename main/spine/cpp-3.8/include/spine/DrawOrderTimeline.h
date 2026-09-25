
#ifndef Spine_DrawOrderTimeline_h
#define Spine_DrawOrderTimeline_h

#include <spine/Timeline.h>

namespace spine {
	class SP_API DrawOrderTimeline : public Timeline {
		friend class SkeletonBinary;
		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit DrawOrderTimeline(int frameCount);

		virtual void apply(Skeleton& skeleton, float lastTime, float time, Vector<Event*>* pEvents, float alpha, MixBlend blend, MixDirection direction);

		virtual int getPropertyId();

		void setFrame(size_t frameIndex, float time, Vector<int>& drawOrder);

		Vector<float>& getFrames();
		Vector< Vector<int> >& getDrawOrders();
		size_t getFrameCount();

	private:
		Vector<float> _frames;
		Vector< Vector<int> > _drawOrders;
	};
}

#endif
