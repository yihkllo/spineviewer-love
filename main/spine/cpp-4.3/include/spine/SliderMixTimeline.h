#ifndef Spine_SliderMixTimeline_h
#define Spine_SliderMixTimeline_h

#include <spine/ConstraintTimeline1.h>

namespace spine {

	class SP_API SliderMixTimeline : public ConstraintTimeline1 {
		friend class SkeletonBinary;
		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit SliderMixTimeline(size_t frameCount, size_t bezierCount, int sliderIndex);

		virtual ~SliderMixTimeline();

		virtual void apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
						   bool appliedPose) override;
	};
}

#endif
