
#ifndef Spine_PathConstraintData_h
#define Spine_PathConstraintData_h

#include <spine/PositionMode.h>
#include <spine/SpacingMode.h>
#include <spine/RotateMode.h>
#include <spine/Vector.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>
#include <spine/ConstraintData.h>

namespace spine {
	class BoneData;
	class SlotData;

	class SP_API PathConstraintData : public ConstraintData {
		friend class SkeletonBinary;
		friend class SkeletonJson;

		friend class PathConstraint;
		friend class Skeleton;
		friend class PathConstraintMixTimeline;
		friend class PathConstraintPositionTimeline;
		friend class PathConstraintSpacingTimeline;

	public:
		explicit PathConstraintData(const String& name);

		Vector<BoneData*>& getBones();

		SlotData* getTarget();
		void setTarget(SlotData* inValue);

		PositionMode getPositionMode();
		void setPositionMode(PositionMode inValue);

		SpacingMode getSpacingMode();
		void setSpacingMode(SpacingMode inValue);

		RotateMode getRotateMode();
		void setRotateMode(RotateMode inValue);

		float getOffsetRotation();
		void setOffsetRotation(float inValue);

		float getPosition();
		void setPosition(float inValue);

		float getSpacing();
		void setSpacing(float inValue);

		float getRotateMix();
		void setRotateMix(float inValue);

		float getTranslateMix();
		void setTranslateMix(float inValue);

	private:
		Vector<BoneData*> _bones;
		SlotData* _target;
		PositionMode _positionMode;
		SpacingMode _spacingMode;
		RotateMode _rotateMode;
		float _offsetRotation;
		float _position, _spacing, _rotateMix, _translateMix;
	};
}

#endif
