#ifndef Spine_ScaleTimeline_h
#define Spine_ScaleTimeline_h

#include <spine/BoneTimeline.h>

namespace spine {

	class SP_API ScaleTimeline : public BoneTimeline2 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit ScaleTimeline(size_t frameCount, size_t bezierCount, int boneIndex);

	protected:
		virtual void _apply(BonePose &pose, BonePose &setup, float time, float alpha, MixFrom from, bool add, bool out) override;
	};

	class SP_API ScaleXTimeline : public BoneTimeline1 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit ScaleXTimeline(size_t frameCount, size_t bezierCount, int boneIndex);

	protected:
		virtual void _apply(BonePose &pose, BonePose &setup, float time, float alpha, MixFrom from, bool add, bool out) override;
	};

	class SP_API ScaleYTimeline : public BoneTimeline1 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit ScaleYTimeline(size_t frameCount, size_t bezierCount, int boneIndex);

	protected:
		virtual void _apply(BonePose &pose, BonePose &setup, float time, float alpha, MixFrom from, bool add, bool out) override;
	};
}

#endif
