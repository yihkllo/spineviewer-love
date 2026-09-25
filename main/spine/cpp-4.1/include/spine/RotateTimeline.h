
#ifndef Spine_RotateTimeline_h
#define Spine_RotateTimeline_h

#include <spine/CurveTimeline.h>

namespace spine {
	class SP_API RotateTimeline : public CurveTimeline1 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		friend class AnimationState;

	RTTI_DECL

	public:
		explicit RotateTimeline(size_t frameCount, size_t bezierCount, int boneIndex);

		virtual void
		apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha, MixBlend blend,
			  MixDirection direction);

		int getBoneIndex() { return _boneIndex; }

		void setBoneIndex(int inValue) { _boneIndex = inValue; }

	private:
		int _boneIndex;
	};
}

#endif
