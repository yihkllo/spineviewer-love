#ifndef Spine_IkConstraintTimeline_h
#define Spine_IkConstraintTimeline_h

#include <spine/CurveTimeline.h>
#include <spine/ConstraintTimeline.h>

namespace spine {

	class SP_API IkConstraintTimeline : public CurveTimeline, public ConstraintTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit IkConstraintTimeline(size_t frameCount, size_t bezierCount, int constraintIndex);

		virtual ~IkConstraintTimeline();

		virtual void apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
						   bool appliedPose) override;

		void setFrame(int frame, float time, float mix, float softness, int bendDirection, bool compress, bool stretch);

		virtual int getConstraintIndex() const override {
			return _constraintIndex;
		}

		virtual void setConstraintIndex(int inValue) override {
			_constraintIndex = inValue;
		}

	private:
		int _constraintIndex;

		static const int ENTRIES = 6;
		static const int MIX = 1;
		static const int SOFTNESS = 2;
		static const int BEND_DIRECTION = 3;
		static const int COMPRESS = 4;
		static const int STRETCH = 5;
	};
}

#endif
