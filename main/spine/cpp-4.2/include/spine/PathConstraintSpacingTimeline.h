
#ifndef Spine_PathConstraintSpacingTimeline_h
#define Spine_PathConstraintSpacingTimeline_h

#include <spine/PathConstraintPositionTimeline.h>

namespace spine {
	class SP_API PathConstraintSpacingTimeline : public CurveTimeline1 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit PathConstraintSpacingTimeline(size_t frameCount, size_t bezierCount, int pathConstraintIndex);

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction);

		int getPathConstraintIndex() { return _pathConstraintIndex; }

		void setPathConstraintIndex(int inValue) { _pathConstraintIndex = inValue; }

	protected:
		int _pathConstraintIndex;
	};
}

#endif
