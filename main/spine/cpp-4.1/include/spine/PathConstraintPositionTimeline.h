
#ifndef Spine_PathConstraintPositionTimeline_h
#define Spine_PathConstraintPositionTimeline_h

#include <spine/CurveTimeline.h>

namespace spine {

	class SP_API PathConstraintPositionTimeline : public CurveTimeline1 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		static const int ENTRIES;

		explicit PathConstraintPositionTimeline(size_t frameCount, size_t bezierCount, int pathConstraintIndex);

		virtual ~PathConstraintPositionTimeline();

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
