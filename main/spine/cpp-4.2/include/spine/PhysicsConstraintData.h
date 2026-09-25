
#ifndef Spine_PhysicsConstraintData_h
#define Spine_PhysicsConstraintData_h

#include <spine/Vector.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>
#include <spine/ConstraintData.h>

namespace spine {
	class BoneData;

	class SP_API PhysicsConstraintData : public ConstraintData {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		friend class Skeleton;

        friend class PhysicsConstraint;

	public:
		RTTI_DECL

		explicit PhysicsConstraintData(const String &name);

        void setBone(BoneData* bone);

        BoneData* getBone() const;

        void setX(float x);

        float getX() const;

        void setY(float y);

        float getY() const;

        void setRotate(float rotate);

        float getRotate() const;

        void setScaleX(float scaleX);

        float getScaleX() const;

        void setShearX(float shearX);

        float getShearX() const;

        void setLimit(float limit);

        float getLimit() const;

        void setStep(float step);

        float getStep() const;

        void setInertia(float inertia);

        float getInertia() const;

        void setStrength(float strength);

        float getStrength() const;

        void setDamping(float damping);

        float getDamping() const;

        void setMassInverse(float massInverse);

        float getMassInverse() const;

        void setWind(float wind);

        float getWind() const;

        void setGravity(float gravity);

        float getGravity() const;

        void setMix(float mix);

        float getMix() const;

        void setInertiaGlobal(bool inertiaGlobal);

        bool isInertiaGlobal() const;

        void setStrengthGlobal(bool strengthGlobal);

        bool isStrengthGlobal() const;

        void setDampingGlobal(bool dampingGlobal);

        bool isDampingGlobal() const;

        void setMassGlobal(bool massGlobal);

        bool isMassGlobal() const;

        void setWindGlobal(bool windGlobal);

        bool isWindGlobal() const;

        void setGravityGlobal(bool gravityGlobal);

        bool isGravityGlobal() const;

        void setMixGlobal(bool mixGlobal);

        bool isMixGlobal() const;

	private:
		BoneData *_bone;
        float _x, _y, _rotate, _scaleX, _shearX, _limit;
        float _step, _inertia, _strength, _damping, _massInverse, _wind, _gravity, _mix;
        bool _inertiaGlobal, _strengthGlobal, _dampingGlobal, _massGlobal, _windGlobal, _gravityGlobal, _mixGlobal;
	};
}

#endif
