#ifndef Spine_BoneTimeline_h
#define Spine_BoneTimeline_h

#include <cstddef>
#include <spine/dll.h>
#include <spine/CurveTimeline.h>

namespace spine {
	class Skeleton;
	class Event;
	class BonePose;

	class SP_API BoneTimeline {
		RTTI_DECL_NOPARENT

	public:
		BoneTimeline(int boneIndex) {
		}
		virtual ~BoneTimeline() {
		}

		virtual int getBoneIndex() const = 0;

		virtual void setBoneIndex(int inValue) = 0;
	};

	class SP_API BoneTimeline1 : public CurveTimeline1, public BoneTimeline {
		friend class SkeletonBinary;
		friend class SkeletonJson;
		friend class AnimationState;

		RTTI_DECL

	public:
		BoneTimeline1(size_t frameCount, size_t bezierCount, int boneIndex, Property property);

		virtual void apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
						   bool appliedPose) override;

		virtual int getBoneIndex() const override {
			return _boneIndex;
		}

		virtual void setBoneIndex(int inValue) override {
			_boneIndex = inValue;
		}

	protected:

		virtual void _apply(BonePose &pose, BonePose &setup, float time, float alpha, MixFrom from, bool add, bool out) = 0;

		int _boneIndex;
	};

	class SP_API BoneTimeline2 : public CurveTimeline, public BoneTimeline {
		friend class SkeletonBinary;
		friend class SkeletonJson;
		friend class AnimationState;

		RTTI_DECL

	public:
		BoneTimeline2(size_t frameCount, size_t bezierCount, int boneIndex, Property property1, Property property2);

		virtual void apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
						   bool appliedPose) override;

		virtual int getBoneIndex() const override {
			return _boneIndex;
		}

		virtual void setBoneIndex(int inValue) override {
			_boneIndex = inValue;
		}

		virtual void setFrame(size_t frame, float time, float value1, float value2);

	protected:

		virtual void _apply(BonePose &pose, BonePose &setup, float time, float alpha, MixFrom from, bool add, bool out) = 0;

		int _boneIndex;

		static const int ENTRIES;
		static const int VALUE1;
		static const int VALUE2;
	};
}

#endif
