#ifndef Spine_RotateTimeline_h
#define Spine_RotateTimeline_h

#include <spine/BoneTimeline.h>

namespace spine {

	class SP_API RotateTimeline : public BoneTimeline1 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		friend class AnimationState;

		RTTI_DECL

	public:
		explicit RotateTimeline(size_t frameCount, size_t bezierCount, int boneIndex);

	protected:
		virtual void _apply(BonePose &pose, BonePose &setup, float time, float alpha, MixFrom from, bool add, bool out) override;
	};
}

#endif
