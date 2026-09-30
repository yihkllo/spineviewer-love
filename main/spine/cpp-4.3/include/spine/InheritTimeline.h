#ifndef Spine_InheritTimeline_h
#define Spine_InheritTimeline_h

#include <spine/Timeline.h>
#include <spine/BoneTimeline.h>
#include <spine/Animation.h>
#include <spine/Property.h>
#include <spine/Inherit.h>

namespace spine {

	class SP_API InheritTimeline : public Timeline, public BoneTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit InheritTimeline(size_t frameCount, int boneIndex);

		virtual ~InheritTimeline();

		void setFrame(int frame, float time, Inherit inherit);

		virtual void apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
						   bool appliedPose) override;

		virtual int getBoneIndex() const override {
			return _boneIndex;
		}

		void setBoneIndex(int inValue) override {
			_boneIndex = inValue;
		}

	private:
		int _boneIndex;

		static const int ENTRIES = 2;
		static const int INHERIT = 1;
	};
}

#endif
