
#ifndef Spine_TranslateTimeline_h
#define Spine_TranslateTimeline_h

#include <spine/CurveTimeline.h>

#include <spine/Animation.h>
#include <spine/TimelineType.h>

namespace spine {

	class SP_API TranslateTimeline : public CurveTimeline {
		friend class SkeletonBinary;
		friend class SkeletonJson;

		RTTI_DECL

	public:
		static const int ENTRIES;

		explicit TranslateTimeline(int frameCount);

		virtual ~TranslateTimeline();

		virtual void apply(Skeleton& skeleton, float lastTime, float time, Vector<Event*>* pEvents, float alpha, MixBlend blend, MixDirection direction);

		virtual int getPropertyId();

		void setFrame(int frameIndex, float time, float x, float y);

	protected:
		static const int PREV_TIME;
		static const int PREV_X;
		static const int PREV_Y;
		static const int X;
		static const int Y;

		Vector<float> _frames;
		int _boneIndex;
	};
}

#endif
