
#ifndef Spine_RotateTimeline_h
#define Spine_RotateTimeline_h

#include <spine/CurveTimeline.h>

namespace spine {
	class SP_API RotateTimeline : public CurveTimeline {
		friend class SkeletonBinary;
		friend class SkeletonJson;
		friend class AnimationState;

		RTTI_DECL

	public:
		static const int ENTRIES = 2;

		explicit RotateTimeline(int frameCount);

		virtual void apply(Skeleton& skeleton, float lastTime, float time, Vector<Event*>* pEvents, float alpha, MixBlend blend, MixDirection direction);

		virtual int getPropertyId();

		void setFrame(int frameIndex, float time, float degrees);

		int getBoneIndex();
		void setBoneIndex(int inValue);

		Vector<float>& getFrames();

	private:
		static const int PREV_TIME = -2;
		static const int PREV_ROTATION = -1;
		static const int ROTATION = 1;

		int _boneIndex;
		Vector<float> _frames;
	};
}

#endif
