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

		virtual void apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
						   bool appliedPose) override;

		size_t getFrameCount();

		Array<Event *> &getEvents();

		void setFrame(size_t frame, Event &event);

	private:
		Array<Event *> _events;
	};
}

#endif
