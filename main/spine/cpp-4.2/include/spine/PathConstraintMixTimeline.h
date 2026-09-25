
#ifndef Spine_PathConstraintMixTimeline_h
#define Spine_PathConstraintMixTimeline_h

#include <spine/CurveTimeline.h>

namespace spine {

	class SP_API PathConstraintMixTimeline : public CurveTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit PathConstraintMixTimeline(size_t frameCount, size_t bezierCount, int pathConstraintIndex);

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction);

		void setFrame(int frameIndex, float time, float mixRotate, float mixX, float mixY);

		int getPathConstraintIndex() { return _constraintIndex; }

		void setPathConstraintIndex(int inValue) { _constraintIndex = inValue; }

	private:
		int _constraintIndex;

		static const int ENTRIES = 4;
		static const int ROTATE = 1;
		static const int X = 2;
		static const int Y = 3;
	};
}

#endif
