#ifndef Spine_ShearTimeline_h
#define Spine_ShearTimeline_h

#include <spine/BoneTimeline.h>

namespace spine {

	class SP_API ShearTimeline : public BoneTimeline2 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit ShearTimeline(size_t frameCount, size_t bezierCount, int boneIndex);

	protected:
		virtual void _apply(BonePose &pose, BonePose &setup, float time, float alpha, MixFrom from, bool add, bool out) override;
	};

	class SP_API ShearXTimeline : public BoneTimeline1 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit ShearXTimeline(size_t frameCount, size_t bezierCount, int boneIndex);

	protected:
		virtual void _apply(BonePose &pose, BonePose &setup, float time, float alpha, MixFrom from, bool add, bool out) override;
	};

	class SP_API ShearYTimeline : public BoneTimeline1 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit ShearYTimeline(size_t frameCount, size_t bezierCount, int boneIndex);

	protected:
		virtual void _apply(BonePose &pose, BonePose &setup, float time, float alpha, MixFrom from, bool add, bool out) override;
	};
}

#endif
