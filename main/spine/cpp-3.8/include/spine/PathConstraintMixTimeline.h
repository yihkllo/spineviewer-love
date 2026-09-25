
#ifndef Spine_PathConstraintMixTimeline_h
#define Spine_PathConstraintMixTimeline_h

#include <spine/CurveTimeline.h>

namespace spine {
#define SP_PATHCONSTRAINTMIXTIMELINE_ENTRIES 5

	class SP_API PathConstraintMixTimeline : public CurveTimeline {
		friend class SkeletonBinary;
		friend class SkeletonJson;

		RTTI_DECL

	public:
		static const int ENTRIES;

		explicit PathConstraintMixTimeline(int frameCount);

		virtual void apply(Skeleton& skeleton, float lastTime, float time, Vector<Event*>* pEvents, float alpha, MixBlend blend, MixDirection direction);

		virtual int getPropertyId();

	private:
		static const int PREV_TIME;
		static const int PREV_ROTATE;
		static const int PREV_TRANSLATE;
		static const int ROTATE;
		static const int TRANSLATE;

		Vector<float> _frames;
		int _pathConstraintIndex;

		void setFrame(int frameIndex, float time, float rotateMix, float translateMix);
	};
}

#endif
