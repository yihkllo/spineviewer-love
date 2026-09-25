
#ifndef Spine_CurveTimeline_h
#define Spine_CurveTimeline_h

#include <spine/Timeline.h>
#include <spine/Vector.h>

namespace spine {
	class SP_API CurveTimeline : public Timeline {
	RTTI_DECL

	public:
		explicit CurveTimeline(size_t frameCount, size_t frameEntries, size_t bezierCount);

		virtual ~CurveTimeline();

		void setLinear(size_t frame);

		void setStepped(size_t frame);

		virtual void
		setBezier(size_t bezier, size_t frame, float value, float time1, float value1, float cx1, float cy1, float cx2,
				  float cy2, float time2, float value2);

		float getBezierValue(float time, size_t frame, size_t valueOffset, size_t i);

		Vector<float> &getCurves();

	protected:
		static const int LINEAR = 0;
		static const int STEPPED = 1;
		static const int BEZIER = 2;
		static const int BEZIER_SIZE = 18;

		Vector<float> _curves;
	};

	class SP_API CurveTimeline1 : public CurveTimeline {
	RTTI_DECL

	public:
		explicit CurveTimeline1(size_t frameCount, size_t bezierCount);

		virtual ~CurveTimeline1();

		void setFrame(size_t frame, float time, float value);

		float getCurveValue(float time);

	protected:
		static const int ENTRIES = 2;
		static const int VALUE = 1;
	};

	class SP_API CurveTimeline2 : public CurveTimeline {
	RTTI_DECL

	public:
		explicit CurveTimeline2(size_t frameCount, size_t bezierCount);

		virtual ~CurveTimeline2();

		void setFrame(size_t frame, float time, float value1, float value2);

		float getCurveValue(float time);

	protected:
		static const int ENTRIES = 3;
		static const int VALUE1 = 1;
		static const int VALUE2 = 2;
	};
}

#endif
