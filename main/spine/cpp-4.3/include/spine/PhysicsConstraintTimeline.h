#ifndef Spine_PhysicsConstraintTimeline_h
#define Spine_PhysicsConstraintTimeline_h

#include <spine/ConstraintTimeline.h>
#include <spine/CurveTimeline.h>
#include <spine/PhysicsConstraint.h>
#include <spine/PhysicsConstraintData.h>
#include <spine/PhysicsConstraintPose.h>

namespace spine {

	class SP_API PhysicsConstraintTimeline : public CurveTimeline1, public ConstraintTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:

		explicit PhysicsConstraintTimeline(size_t frameCount, size_t bezierCount, int constraintIndex, Property property);

		virtual void apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
						   bool appliedPose) override;

		virtual int getConstraintIndex() const override {
			return _constraintIndex;
		}

		virtual void setConstraintIndex(int inValue) override {
			_constraintIndex = inValue;
		}

	protected:
		virtual float get(PhysicsConstraintPose &pose) = 0;
		virtual void set(PhysicsConstraintPose &pose, float value) = 0;
		virtual bool global(PhysicsConstraintData &constraintData) = 0;

		int _constraintIndex;
	};

	class SP_API PhysicsConstraintInertiaTimeline : public PhysicsConstraintTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit PhysicsConstraintInertiaTimeline(size_t frameCount, size_t bezierCount, int physicsConstraintIndex)
			: PhysicsConstraintTimeline(frameCount, bezierCount, physicsConstraintIndex, Property_PhysicsConstraintInertia) {};

	protected:
		float get(PhysicsConstraintPose &pose) override {
			return pose.getInertia();
		}

		void set(PhysicsConstraintPose &pose, float value) override {
			pose.setInertia(value);
		}

		bool global(PhysicsConstraintData &constraintData) override {
			return constraintData.getInertiaGlobal();
		}
	};

	class SP_API PhysicsConstraintStrengthTimeline : public PhysicsConstraintTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit PhysicsConstraintStrengthTimeline(size_t frameCount, size_t bezierCount, int physicsConstraintIndex)
			: PhysicsConstraintTimeline(frameCount, bezierCount, physicsConstraintIndex, Property_PhysicsConstraintStrength) {};

	protected:
		float get(PhysicsConstraintPose &pose) override {
			return pose.getStrength();
		}

		void set(PhysicsConstraintPose &pose, float value) override {
			pose.setStrength(value);
		}

		bool global(PhysicsConstraintData &constraintData) override {
			return constraintData.getStrengthGlobal();
		}
	};

	class SP_API PhysicsConstraintDampingTimeline : public PhysicsConstraintTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit PhysicsConstraintDampingTimeline(size_t frameCount, size_t bezierCount, int physicsConstraintIndex)
			: PhysicsConstraintTimeline(frameCount, bezierCount, physicsConstraintIndex, Property_PhysicsConstraintDamping) {};

	protected:
		float get(PhysicsConstraintPose &pose) override {
			return pose.getDamping();
		}

		void set(PhysicsConstraintPose &pose, float value) override {
			pose.setDamping(value);
		}

		bool global(PhysicsConstraintData &constraintData) override {
			return constraintData.getDampingGlobal();
		}
	};

	class SP_API PhysicsConstraintMassTimeline : public PhysicsConstraintTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit PhysicsConstraintMassTimeline(size_t frameCount, size_t bezierCount, int physicsConstraintIndex)
			: PhysicsConstraintTimeline(frameCount, bezierCount, physicsConstraintIndex, Property_PhysicsConstraintMass) {};

	protected:
		float get(PhysicsConstraintPose &pose) override {
			return 1 / pose.getMassInverse();
		}

		void set(PhysicsConstraintPose &pose, float value) override {
			pose.setMassInverse(1 / value);
		}

		bool global(PhysicsConstraintData &constraintData) override {
			return constraintData.getMassGlobal();
		}
	};

	class SP_API PhysicsConstraintWindTimeline : public PhysicsConstraintTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit PhysicsConstraintWindTimeline(size_t frameCount, size_t bezierCount, int physicsConstraintIndex)
			: PhysicsConstraintTimeline(frameCount, bezierCount, physicsConstraintIndex, Property_PhysicsConstraintWind) {
			_additive = true;
		};

	protected:
		float get(PhysicsConstraintPose &pose) override {
			return pose.getWind();
		}

		void set(PhysicsConstraintPose &pose, float value) override {
			pose.setWind(value);
		}

		bool global(PhysicsConstraintData &constraintData) override {
			return constraintData.getWindGlobal();
		}
	};

	class SP_API PhysicsConstraintGravityTimeline : public PhysicsConstraintTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit PhysicsConstraintGravityTimeline(size_t frameCount, size_t bezierCount, int physicsConstraintIndex)
			: PhysicsConstraintTimeline(frameCount, bezierCount, physicsConstraintIndex, Property_PhysicsConstraintGravity) {
			_additive = true;
		};

	protected:
		float get(PhysicsConstraintPose &pose) override {
			return pose.getGravity();
		}

		void set(PhysicsConstraintPose &pose, float value) override {
			pose.setGravity(value);
		}

		bool global(PhysicsConstraintData &constraintData) override {
			return constraintData.getGravityGlobal();
		}
	};

	class SP_API PhysicsConstraintMixTimeline : public PhysicsConstraintTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit PhysicsConstraintMixTimeline(size_t frameCount, size_t bezierCount, int physicsConstraintIndex)
			: PhysicsConstraintTimeline(frameCount, bezierCount, physicsConstraintIndex, Property_PhysicsConstraintMix) {};

	protected:
		float get(PhysicsConstraintPose &pose) override {
			return pose.getMix();
		}

		void set(PhysicsConstraintPose &pose, float value) override {
			pose.setMix(value);
		}

		bool global(PhysicsConstraintData &constraintData) override {
			return constraintData.getMixGlobal();
		}
	};

	class SP_API PhysicsConstraintResetTimeline : public Timeline, public ConstraintTimeline {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:

		explicit PhysicsConstraintResetTimeline(size_t frameCount, int constraintIndex)
			: Timeline(frameCount, 1), ConstraintTimeline(), _constraintIndex(constraintIndex) {
			PropertyId ids[] = {((PropertyId) Property_PhysicsConstraintReset) << 32};
			setPropertyIds(ids, 1);
			_instant = true;
		}

		virtual void apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
						   bool appliedPose) override;

		int getFrameCount() {
			return (int) _frames.size();
		}

		virtual int getConstraintIndex() const override {
			return _constraintIndex;
		}

		virtual void setConstraintIndex(int inValue) override {
			_constraintIndex = inValue;
		}

		void setFrame(int frame, float time) {
			_frames[frame] = time;
		}

	private:
		int _constraintIndex;
	};
}

#endif
