#ifndef Spine_SliderTimeline_h
#define Spine_SliderTimeline_h

#include <spine/ConstraintTimeline1.h>

namespace spine {

	class SP_API SliderTimeline : public ConstraintTimeline1 {
		friend class SkeletonBinary;
		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit SliderTimeline(size_t frameCount, size_t bezierCount, int sliderIndex);

		virtual ~SliderTimeline();

		virtual void apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
						   bool appliedPose) override;
	};
}

#endif
