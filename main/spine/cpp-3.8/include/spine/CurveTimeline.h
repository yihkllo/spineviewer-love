
#ifndef Spine_CurveTimeline_h
#define Spine_CurveTimeline_h

#include <spine/Timeline.h>
#include <spine/Vector.h>

namespace spine {
	class SP_API CurveTimeline : public Timeline {
		RTTI_DECL

	public:
		explicit CurveTimeline(int frameCount);

		virtual ~CurveTimeline();

		virtual void apply(Skeleton& skeleton, float lastTime, float time, Vector<Event*>* pEvents, float alpha, MixBlend blend, MixDirection direction) = 0;

		virtual int getPropertyId() = 0;

		size_t getFrameCount();

		void setLinear(size_t frameIndex);

		void setStepped(size_t frameIndex);

		void setCurve(size_t frameIndex, float cx1, float cy1, float cx2, float cy2);

		float getCurvePercent(size_t frameIndex, float percent);

		float getCurveType(size_t frameIndex);

	protected:
		static const float LINEAR;
		static const float STEPPED;
		static const float BEZIER;
		static const int BEZIER_SIZE;

	private:
		Vector<float> _curves;
	};
}

#endif
