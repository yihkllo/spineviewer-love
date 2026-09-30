#ifndef Spine_PhysicsConstraintData_h
#define Spine_PhysicsConstraintData_h

#include <spine/ConstraintData.h>
#include <spine/PosedData.h>
#include <spine/PhysicsConstraintPose.h>

namespace spine {
	class BoneData;
	class PhysicsConstraint;

	class SP_API PhysicsConstraintData : public ConstraintDataGeneric<PhysicsConstraint, PhysicsConstraintPose> {
		friend class SkeletonBinary;
		friend class SkeletonJson;
		friend class PhysicsConstraint;
		friend class Skeleton;

		RTTI_DECL
	public:
		explicit PhysicsConstraintData(const String &name);

		virtual Constraint &create(Skeleton &skeleton) override;

		BoneData &getBone();
		void setBone(BoneData &bone);

		float getStep();
		void setStep(float step);

		float getX();
		void setX(float x);

		float getY();
		void setY(float y);

		float getRotate();
		void setRotate(float rotate);

		float getScaleX();
		void setScaleX(float scaleX);

		float getShearX();
		void setShearX(float shearX);

		float getLimit();
		void setLimit(float limit);

		ScaleYMode getScaleYMode();
		void setScaleYMode(ScaleYMode scaleYMode);

		bool getInertiaGlobal();
		void setInertiaGlobal(bool inertiaGlobal);

		bool getStrengthGlobal();
		void setStrengthGlobal(bool strengthGlobal);

		bool getDampingGlobal();
		void setDampingGlobal(bool dampingGlobal);

		bool getMassGlobal();
		void setMassGlobal(bool massGlobal);

		bool getWindGlobal();
		void setWindGlobal(bool windGlobal);

		bool getGravityGlobal();
		void setGravityGlobal(bool gravityGlobal);

		bool getMixGlobal();
		void setMixGlobal(bool mixGlobal);

	private:
		BoneData *_bone;
		float _x, _y, _rotate, _scaleX, _shearX, _limit, _step;
		ScaleYMode _scaleYMode;
		bool _inertiaGlobal, _strengthGlobal, _dampingGlobal, _massGlobal, _windGlobal, _gravityGlobal, _mixGlobal;
	};
}

#endif
