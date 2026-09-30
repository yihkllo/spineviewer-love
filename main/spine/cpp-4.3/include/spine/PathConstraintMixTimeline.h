#ifndef Spine_PathConstraintMixTimeline_h
#define Spine_PathConstraintMixTimeline_h

#include <spine/CurveTimeline.h>
#include <spine/ConstraintTimeline.h>

namespace spine {

	class SP_API PathConstraintMixTimeline : public CurveTimeline, public ConstraintTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit PathConstraintMixTimeline(size_t frameCount, size_t bezierCount, int constraintIndex);

		virtual ~PathConstraintMixTimeline();

		virtual void apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
						   bool appliedPose) override;

		void setFrame(int frame, float time, float mixRotate, float mixX, float mixY);

		virtual int getConstraintIndex() const override {
			return _constraintIndex;
		}

		virtual void setConstraintIndex(int inValue) override {
			_constraintIndex = inValue;
		}

	private:
		int _constraintIndex;

		static const int ENTRIES = 4;
		static const int ROTATE = 1;
		static const int X = 2;
		static const int Y = 3;
	};
}

#endif
