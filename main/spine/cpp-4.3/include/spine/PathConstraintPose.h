#ifndef Spine_PathConstraintPose_h
#define Spine_PathConstraintPose_h

#include <spine/Pose.h>
#include <spine/RTTI.h>

namespace spine {

	class SP_API PathConstraintPose : public Pose<PathConstraintPose> {
		friend class PathConstraint;
		friend class PathConstraintPositionTimeline;
		friend class PathConstraintSpacingTimeline;
		friend class PathConstraintMixTimeline;
		friend class SkeletonJson;
		friend class SkeletonBinary;

	private:
		float _position;
		float _spacing;
		float _mixRotate;
		float _mixX;
		float _mixY;

	public:
		PathConstraintPose();
		virtual ~PathConstraintPose();

		virtual void set(PathConstraintPose &pose) override;

		float getPosition();
		void setPosition(float position);

		float getSpacing();
		void setSpacing(float spacing);

		float getMixRotate();
		void setMixRotate(float mixRotate);

		float getMixX();
		void setMixX(float mixX);

		float getMixY();
		void setMixY(float mixY);
	};
}

#endif
