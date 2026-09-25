
#ifndef Spine_IkConstraintTimeline_h
#define Spine_IkConstraintTimeline_h

#include <spine/CurveTimeline.h>

namespace spine {

	class SP_API IkConstraintTimeline : public CurveTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit IkConstraintTimeline(size_t frameCount, size_t bezierCount, int ikConstraintIndex);

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction);

		void setFrame(int frame, float time, float mix, float softness, int bendDirection, bool compress, bool stretch);

		int getIkConstraintIndex() { return _ikConstraintIndex; }

		void setIkConstraintIndex(int inValue) { _ikConstraintIndex = inValue; }

	private:
		int _ikConstraintIndex;

		static const int ENTRIES = 6;
		static const int MIX = 1;
		static const int SOFTNESS = 2;
		static const int BEND_DIRECTION = 3;
		static const int COMPRESS = 4;
		static const int STRETCH = 5;
	};
}

#endif
