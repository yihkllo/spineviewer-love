
#ifndef Spine_EventTimeline_h
#define Spine_EventTimeline_h

#include <spine/Timeline.h>

namespace spine {
	class SP_API EventTimeline : public Timeline {
		friend class SkeletonBinary;
		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit EventTimeline(int frameCount);

		~EventTimeline();

		virtual void apply(Skeleton& skeleton, float lastTime, float time, Vector<Event*>* pEvents, float alpha, MixBlend blend, MixDirection direction);

		virtual int getPropertyId();

		void setFrame(size_t frameIndex, Event* event);

		const Vector<float>& getFrames();
		Vector<Event*>& getEvents();
		size_t getFrameCount();

	private:
		Vector<float> _frames;
		Vector<Event*> _events;
	};
}

#endif
