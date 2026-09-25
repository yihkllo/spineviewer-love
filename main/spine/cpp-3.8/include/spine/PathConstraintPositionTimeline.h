
#ifndef Spine_PathConstraintPositionTimeline_h
#define Spine_PathConstraintPositionTimeline_h

#include <spine/CurveTimeline.h>

namespace spine {

	class SP_API PathConstraintPositionTimeline : public CurveTimeline {
		friend class SkeletonBinary;
		friend class SkeletonJson;

		RTTI_DECL

	public:
		static const int ENTRIES;

		explicit PathConstraintPositionTimeline(int frameCount);

		virtual ~PathConstraintPositionTimeline();

		virtual void apply(Skeleton& skeleton, float lastTime, float time, Vector<Event*>* pEvents, float alpha, MixBlend blend, MixDirection direction);

		virtual int getPropertyId();

		void setFrame(int frameIndex, float time, float value);

	protected:
		static const int PREV_TIME;
		static const int PREV_VALUE;
		static const int VALUE;

		Vector<float> _frames;
		int _pathConstraintIndex;
	};
}

#endif
