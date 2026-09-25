
#ifndef Spine_TransformConstraint_h
#define Spine_TransformConstraint_h

#include <spine/ConstraintData.h>

#include <spine/Vector.h>

namespace spine {
	class TransformConstraintData;

	class Skeleton;

	class Bone;

	class SP_API TransformConstraint : public Updatable {
		friend class Skeleton;

		friend class TransformConstraintTimeline;

	RTTI_DECL

	public:
		TransformConstraint(TransformConstraintData &data, Skeleton &skeleton);

		virtual void update();

		virtual int getOrder();

		TransformConstraintData &getData();

		Vector<Bone *> &getBones();

		Bone *getTarget();

		void setTarget(Bone *inValue);

		float getMixRotate();

		void setMixRotate(float inValue);

		float getMixX();

		void setMixX(float inValue);

		float getMixY();

		void setMixY(float inValue);

		float getMixScaleX();

		void setMixScaleX(float inValue);

		float getMixScaleY();

		void setMixScaleY(float inValue);

		float getMixShearY();

		void setMixShearY(float inValue);

		bool isActive();

		void setActive(bool inValue);

	private:
		TransformConstraintData &_data;
		Vector<Bone *> _bones;
		Bone *_target;
		float _mixRotate, _mixX, _mixY, _mixScaleX, _mixScaleY, _mixShearY;
		bool _active;

		void applyAbsoluteWorld();

		void applyRelativeWorld();

		void applyAbsoluteLocal();

		void applyRelativeLocal();
	};
}

#endif
