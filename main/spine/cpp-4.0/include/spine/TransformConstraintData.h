
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
		explicit TransformConstraintData(const String &name);

		Vector<BoneData *> &getBones();

		BoneData *getTarget();

		float getMixRotate();

		float getMixX();

		float getMixY();

		float getMixScaleX();

		float getMixScaleY();

		float getMixShearY();

		float getOffsetRotation();

		float getOffsetX();

		float getOffsetY();

		float getOffsetScaleX();

		float getOffsetScaleY();

		float getOffsetShearY();

		bool isRelative();

		bool isLocal();

	private:
		Vector<BoneData *> _bones;
		BoneData *_target;
		float _mixRotate, _mixX, _mixY, _mixScaleX, _mixScaleY, _mixShearY;
		float _offsetRotation, _offsetX, _offsetY, _offsetScaleX, _offsetScaleY, _offsetShearY;
		bool _relative, _local;
	};
}

#endif
