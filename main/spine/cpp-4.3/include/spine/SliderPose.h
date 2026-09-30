#ifndef Spine_SliderPose_h
#define Spine_SliderPose_h

#include <spine/Pose.h>
#include <spine/RTTI.h>

namespace spine {
	class Slider;

	class SP_API SliderPose : public Pose<SliderPose> {
		friend class Slider;
		friend class SliderMixTimeline;
		friend class SliderTimeline;
		friend class SkeletonJson;
		friend class SkeletonBinary;

	private:
		float _time, _mix;

	public:
		SliderPose();
		virtual ~SliderPose();

		virtual void set(SliderPose &pose) override;

		float getTime();
		void setTime(float time);

		float getMix();
		void setMix(float mix);
	};
}

#endif
