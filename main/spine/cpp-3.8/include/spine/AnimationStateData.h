
#ifndef Spine_AnimationStateData_h
#define Spine_AnimationStateData_h

#include <spine/HashMap.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>

#include <assert.h>

namespace spine {
	class SkeletonData;
	class Animation;

	class SP_API AnimationStateData : public SpineObject {
		friend class AnimationState;

	public:
		explicit AnimationStateData(SkeletonData* skeletonData);

		SkeletonData* getSkeletonData();

		float getDefaultMix();
		void setDefaultMix(float inValue);

		void setMix(const String& fromName, const String& toName, float duration);

		void setMix(Animation* from, Animation* to, float duration);

		float getMix(Animation* from, Animation* to);

	private:
		class AnimationPair : public SpineObject {
		public:
			Animation* _a1;
			Animation* _a2;

			explicit AnimationPair(Animation* a1 = NULL, Animation* a2 = NULL);

			bool operator==(const AnimationPair &other) const;
		};

		SkeletonData* _skeletonData;
		float _defaultMix;
		HashMap<AnimationPair, float> _animationToMixTime;
	};
}

#endif
