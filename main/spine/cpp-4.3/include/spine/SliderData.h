#ifndef Spine_SliderData_h
#define Spine_SliderData_h

#include <spine/ConstraintData.h>
#include <spine/PosedData.h>
#include <spine/SliderPose.h>
#include <spine/SpineString.h>

namespace spine {
	class Animation;
	class BoneData;
	class FromProperty;
	class Slider;
	class Skeleton;

	class SP_API SliderData : public ConstraintDataGeneric<Slider, SliderPose> {
		friend class SkeletonBinary;
		friend class SkeletonData;
		friend class SkeletonJson;
		friend class Slider;
		friend class SliderMixTimeline;
		friend class SliderTimeline;

		RTTI_DECL

	public:
		explicit SliderData(const String &name);
		~SliderData();

		virtual Constraint &create(Skeleton &skeleton) override;

		Animation &getAnimation();
		void setAnimation(Animation &animation);

		bool getAdditive();
		void setAdditive(bool additive);

		bool getLoop();
		void setLoop(bool loop);

		BoneData *getBone();
		void setBone(BoneData *bone);

		FromProperty *getProperty();
		void setProperty(FromProperty *property);

		float getScale();
		void setScale(float scale);

		float getOffset();
		void setOffset(float offset);

		float getMax();
		void setMax(float max);

		bool getLocal();
		void setLocal(bool local);

	private:
		Animation *_animation;
		bool _additive;
		bool _loop;
		BoneData *_bone;
		FromProperty *_property;
		float _offset;
		float _scale;
		float _max;
		bool _local;
	};
}

#endif
