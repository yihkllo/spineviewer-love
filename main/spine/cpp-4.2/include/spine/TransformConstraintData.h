
#ifndef Spine_TransformConstraintData_h
#define Spine_TransformConstraintData_h

#include <spine/Vector.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>
#include <spine/ConstraintData.h>

namespace spine {
	class BoneData;

	class SP_API TransformConstraintData : public ConstraintData {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		friend class TransformConstraint;

		friend class Skeleton;

		friend class TransformConstraintTimeline;

	public:
		RTTI_DECL

		explicit TransformConstraintData(const String &name);

		Vector<BoneData *> &getBones();

		BoneData *getTarget();

        void setTarget(BoneData *target);

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

		float getOffsetRotation();

        void setOffsetRotation(float offsetRotation);

		float getOffsetX();

        void setOffsetX(float offsetX);

		float getOffsetY();

        void setOffsetY(float offsetY);

		float getOffsetScaleX();

        void setOffsetScaleX(float offsetScaleX);

		float getOffsetScaleY();

        void setOffsetScaleY(float offsetScaleY);

		float getOffsetShearY();

        void setOffsetShearY(float offsetShearY);

		bool isRelative();

        void setRelative(bool isRelative);

		bool isLocal();

        void setLocal(bool isLocal);

	private:
		Vector<BoneData *> _bones;
		BoneData *_target;
		float _mixRotate, _mixX, _mixY, _mixScaleX, _mixScaleY, _mixShearY;
		float _offsetRotation, _offsetX, _offsetY, _offsetScaleX, _offsetScaleY, _offsetShearY;
		bool _relative, _local;
	};
}

#endif
