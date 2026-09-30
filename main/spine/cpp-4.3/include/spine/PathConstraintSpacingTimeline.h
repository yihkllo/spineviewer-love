#ifndef Spine_PathConstraintSpacingTimeline_h
#define Spine_PathConstraintSpacingTimeline_h

#include <spine/ConstraintTimeline1.h>

namespace spine {

	class SP_API PathConstraintSpacingTimeline : public ConstraintTimeline1 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit PathConstraintSpacingTimeline(size_t frameCount, size_t bezierCount, int constraintIndex);

		virtual ~PathConstraintSpacingTimeline();

		virtual void apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
						   bool appliedPose) override;
	};
}

#endif
