#ifndef Spine_TransformConstraintTimeline_h
#define Spine_TransformConstraintTimeline_h

#include <spine/CurveTimeline.h>
#include <spine/ConstraintTimeline.h>

namespace spine {

	class SP_API TransformConstraintTimeline : public CurveTimeline, public ConstraintTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit TransformConstraintTimeline(size_t frameCount, size_t bezierCount, int transformConstraintIndex);

		virtual ~TransformConstraintTimeline();

		virtual void apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
						   bool appliedPose) override;

		void setFrame(int frame, float time, float mixRotate, float mixX, float mixY, float mixScaleX, float mixScaleY, float mixShearY);

		virtual int getConstraintIndex() const override {
			return _constraintIndex;
		}

		virtual void setConstraintIndex(int inValue) override {
			_constraintIndex = inValue;
		}

	private:
		int _constraintIndex;

		static const int ENTRIES = 7;
		static const int ROTATE = 1;
		static const int X = 2;
		static const int Y = 3;
		static const int SCALEX = 4;
		static const int SCALEY = 5;
		static const int SHEARY = 6;
	};
}

#endif
