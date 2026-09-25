
#ifndef Spine_TransformConstraintTimeline_h
#define Spine_TransformConstraintTimeline_h

#include <spine/CurveTimeline.h>

namespace spine {

	class SP_API TransformConstraintTimeline : public CurveTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit TransformConstraintTimeline(size_t frameCount, size_t bezierCount, int transformConstraintIndex);

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction);

		void setFrame(size_t frameIndex, float time, float mixRotate, float mixX, float mixY, float mixScaleX,
					  float mixScaleY, float mixShearY);

		int getTransformConstraintIndex() { return _constraintIndex; }

		void setTransformConstraintIndex(int inValue) { _constraintIndex = inValue; }

	private:
		int _constraintIndex;

		static const int ENTRIES = 7;
		static const int ROTATE = 1;
		static const int X = 2;
		static const int Y = 3;
		static const int SCALEX = 4;
		static const int SCALEY = 5;
		static const int SHEARY = 6;
	};
}

#endif
