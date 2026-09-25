
#ifndef Spine_EventTimeline_h
#define Spine_EventTimeline_h

#include <spine/Timeline.h>

namespace spine {
	class SP_API EventTimeline : public Timeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit EventTimeline(size_t frameCount);

		~EventTimeline();

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction);

		void setFrame(size_t frame, Event *event);

		Vector<Event *> &getEvents();

	private:
		Vector<Event *> _events;
	};
}

#endif
