
#ifndef Spine_ShearTimeline_h
#define Spine_ShearTimeline_h

#include <spine/TranslateTimeline.h>

namespace spine {
	class SP_API ShearTimeline : public TranslateTimeline {
		friend class SkeletonBinary;
		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit ShearTimeline(int frameCount);

		virtual void apply(Skeleton& skeleton, float lastTime, float time, Vector<Event*>* pEvents, float alpha, MixBlend blend, MixDirection direction);

		virtual int getPropertyId();
	};
}

#endif
