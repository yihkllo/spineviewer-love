#ifndef Spine_PathConstraintPositionTimeline_h
#define Spine_PathConstraintPositionTimeline_h

#include <spine/ConstraintTimeline1.h>

namespace spine {

	class SP_API PathConstraintPositionTimeline : public ConstraintTimeline1 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		static const int ENTRIES;

		explicit PathConstraintPositionTimeline(size_t frameCount, size_t bezierCount, int constraintIndex);

		virtual ~PathConstraintPositionTimeline();

		virtual void apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
						   bool appliedPose) override;
	};
}

#endif
