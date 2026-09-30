#ifndef Spine_TransformConstraintPose_h
#define Spine_TransformConstraintPose_h

#include <spine/Pose.h>
#include <spine/RTTI.h>

namespace spine {

	class SP_API TransformConstraintPose : public Pose<TransformConstraintPose> {
		friend class FromProperty;
		friend class ToProperty;
		friend class FromRotate;
		friend class ToRotate;
		friend class FromX;
		friend class ToX;
		friend class FromY;
		friend class ToY;
		friend class FromScaleX;
		friend class ToScaleX;
		friend class FromScaleY;
		friend class ToScaleY;
		friend class FromShearY;
		friend class ToShearY;
		friend class TransformConstraint;
		friend class TransformConstraintTimeline;
		friend class SkeletonJson;
		friend class SkeletonBinary;

	private:
		float _mixRotate, _mixX, _mixY, _mixScaleX, _mixScaleY, _mixShearY;

	public:
		TransformConstraintPose();
		virtual ~TransformConstraintPose();

		virtual void set(TransformConstraintPose &pose) override;

		float getMixRotate();
		void setMixRotate(float mixRotate);

		float getMixX();
		void setMixX(float mixX);

		float getMixY();
		void setMixY(float mixY);

		float getMixScaleX();
		void setMixScaleX(float mixScaleX);

		float getMixScaleY();
		void setMixScaleY(float mixScaleY);

		float getMixShearY();
		void setMixShearY(float mixShearY);
	};
}

#endif
