
#ifndef Spine_IkConstraintTimeline_h
#define Spine_IkConstraintTimeline_h

#include <spine/CurveTimeline.h>

namespace spine {

	class SP_API IkConstraintTimeline : public CurveTimeline {
		friend class SkeletonBinary;
		friend class SkeletonJson;

		RTTI_DECL

	public:
		static const int ENTRIES;

		explicit IkConstraintTimeline(int frameCount);

		virtual void apply(Skeleton& skeleton, float lastTime, float time, Vector<Event*>* pEvents, float alpha, MixBlend blend, MixDirection direction);

		virtual int getPropertyId();

		void setFrame (int frameIndex, float time, float mix, float softness, int bendDirection, bool compress, bool stretch);

	private:
		static const int PREV_TIME;
		static const int PREV_MIX;
		static const int PREV_SOFTNESS;
		static const int PREV_BEND_DIRECTION;
		static const int PREV_COMPRESS;
		static const int PREV_STRETCH;
		static const int MIX;
		static const int SOFTNESS;
		static const int BEND_DIRECTION;
		static const int COMPRESS;
		static const int STRETCH;

		Vector<float> _frames;
		int _ikConstraintIndex;
	};
}

#endif
