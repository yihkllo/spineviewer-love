#ifndef Spine_Slider_h
#define Spine_Slider_h

#include <spine/Constraint.h>
#include <spine/SliderData.h>
#include <spine/SliderPose.h>

namespace spine {
	class Skeleton;
	class Bone;
	class Animation;

	class SliderBase : public ConstraintGeneric<Slider, SliderData, SliderPose> {
	public:
		SliderBase(SliderData &data) : ConstraintGeneric<Slider, SliderData, SliderPose>(data) {
		}
	};

	class SP_API Slider : public SliderBase {
		friend class Skeleton;
		friend class SliderTimeline;
		friend class SliderMixTimeline;

		RTTI_DECL

	public:
		Slider(SliderData &data, Skeleton &skeleton);

		Slider &copy(Skeleton &skeleton);

		virtual void update(Skeleton &skeleton, Physics physics) override;

		virtual void sort(Skeleton &skeleton) override;

		virtual bool isSourceActive() override;

		Bone &getBone();

		void setBone(Bone &bone);

	private:
		Bone *_bone;
		static float _offsets[6];
	};
}

#endif
