
#ifndef Spine_TransformConstraintTimeline_h
#define Spine_TransformConstraintTimeline_h

#include <spine/CurveTimeline.h>

namespace spine {

	class SP_API TransformConstraintTimeline : public CurveTimeline {
		friend class SkeletonBinary;
		friend class SkeletonJson;

		RTTI_DECL

	public:
		static const int ENTRIES;

		explicit TransformConstraintTimeline(int frameCount);

		virtual void apply(Skeleton& skeleton, float lastTime, float time, Vector<Event*>* pEvents, float alpha, MixBlend blend, MixDirection direction);

		virtual int getPropertyId();

		void setFrame(size_t frameIndex, float time, float rotateMix, float translateMix, float scaleMix, float shearMix);

	private:
		static const int PREV_TIME;
		static const int PREV_ROTATE;
		static const int PREV_TRANSLATE;
		static const int PREV_SCALE;
		static const int PREV_SHEAR;
		static const int ROTATE;
		static const int TRANSLATE;
		static const int SCALE;
		static const int SHEAR;

		Vector<float> _frames;
		int _transformConstraintIndex;
	};
}

#endif
