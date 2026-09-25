
#ifndef Spine_TwoColorTimeline_h
#define Spine_TwoColorTimeline_h

#include <spine/CurveTimeline.h>

namespace spine {

	class SP_API TwoColorTimeline : public CurveTimeline {
		friend class SkeletonBinary;
		friend class SkeletonJson;

		RTTI_DECL

	public:
		static const int ENTRIES;

		explicit TwoColorTimeline(int frameCount);

		virtual void apply(Skeleton& skeleton, float lastTime, float time, Vector<Event*>* pEvents, float alpha, MixBlend blend, MixDirection direction);

		virtual int getPropertyId();

		void setFrame(int frameIndex, float time, float r, float g, float b, float a, float r2, float g2, float b2);

		int getSlotIndex();
		void setSlotIndex(int inValue);

	private:
		static const int PREV_TIME;
		static const int PREV_R;
		static const int PREV_G;
		static const int PREV_B;
		static const int PREV_A;
		static const int PREV_R2;
		static const int PREV_G2;
		static const int PREV_B2;
		static const int R;
		static const int G;
		static const int B;
		static const int A;
		static const int R2;
		static const int G2;
		static const int B2;

		Vector<float> _frames;
		int _slotIndex;
	};
}

#endif
