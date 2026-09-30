#ifndef Spine_TranslateTimeline_h
#define Spine_TranslateTimeline_h

#include <spine/BoneTimeline.h>

namespace spine {

	class SP_API TranslateTimeline : public BoneTimeline2 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit TranslateTimeline(size_t frameCount, size_t bezierCount, int boneIndex);

	protected:
		virtual void _apply(BonePose &pose, BonePose &setup, float time, float alpha, MixFrom from, bool add, bool out) override;
	};

	class SP_API TranslateXTimeline : public BoneTimeline1 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit TranslateXTimeline(size_t frameCount, size_t bezierCount, int boneIndex);

	protected:
		virtual void _apply(BonePose &pose, BonePose &setup, float time, float alpha, MixFrom from, bool add, bool out) override;
	};

	class SP_API TranslateYTimeline : public BoneTimeline1 {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit TranslateYTimeline(size_t frameCount, size_t bezierCount, int boneIndex);

	protected:
		virtual void _apply(BonePose &pose, BonePose &setup, float time, float alpha, MixFrom from, bool add, bool out) override;
	};
}

#endif
