#ifndef SPINE_BONELOCAL_H_
#define SPINE_BONELOCAL_H_

#include <spine/SpineObject.h>
#include <spine/RTTI.h>
#include <spine/Pose.h>
#include <spine/Inherit.h>

namespace spine {

	class SP_API BoneLocal : public Pose<BoneLocal> {
		friend class IkConstraint;
		friend class BoneTimeline1;
		friend class BoneTimeline2;
		friend class RotateTimeline;
		friend class InheritTimeline;
		friend class ScaleTimeline;
		friend class ScaleXTimeline;
		friend class ScaleYTimeline;
		friend class ShearTimeline;
		friend class ShearXTimeline;
		friend class ShearYTimeline;
		friend class TranslateTimeline;
		friend class TranslateXTimeline;
		friend class TranslateYTimeline;
		friend class SkeletonJson;
		friend class SkeletonBinary;
		friend class Skeleton;
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
		friend class AnimationState;

	protected:
		float _x, _y, _rotation, _scaleX, _scaleY, _shearX, _shearY;
		Inherit _inherit;

	public:
		BoneLocal();
		virtual ~BoneLocal();

		virtual void set(BoneLocal &pose) override;

		float getX();
		void setX(float x);

		float getY();
		void setY(float y);

		void setPosition(float x, float y);

		float getRotation();
		void setRotation(float rotation);

		float getScaleX();
		void setScaleX(float scaleX);

		float getScaleY();
		void setScaleY(float scaleY);

		void setScale(float scaleX, float scaleY);

		void setScale(float scale);

		float getShearX();
		void setShearX(float shearX);

		float getShearY();
		void setShearY(float shearY);

		Inherit getInherit();
		void setInherit(Inherit inherit);
	};
}

#endif
