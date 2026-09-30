#ifndef Spine_Constraint_h
#define Spine_Constraint_h

#include <spine/Posed.h>
#include <spine/PosedActive.h>
#include <spine/Update.h>
#include <spine/RTTI.h>
#include <spine/ConstraintData.h>

namespace spine {
	class Skeleton;

	class SP_API Constraint : public Update {
		friend class Skeleton;

	public:
		RTTI_DECL

		Constraint();
		virtual ~Constraint();

		virtual ConstraintData &getData() = 0;

		virtual void sort(Skeleton &skeleton) = 0;

		virtual bool isSourceActive() = 0;

		virtual void update(Skeleton &skeleton, Physics physics) override = 0;

	protected:
		virtual void unconstrained() = 0;

		virtual void setupPose() = 0;

		bool _active;
	};

	template<class T, class D, class P>
	class ConstraintGeneric : public PosedGeneric<D, P, P>, public PosedActive, public Constraint {
	public:
		ConstraintGeneric(D &data) : PosedGeneric<D, P, P>(data), PosedActive(), Constraint() {
		}

		virtual ~ConstraintGeneric() {
		}

		virtual D &getData() override {
			return PosedGeneric<D, P, P>::getData();
		}

	protected:
		virtual void unconstrained() override {
			PosedGeneric<D, P, P>::unconstrained();
		}

		virtual void setupPose() override {
			PosedGeneric<D, P, P>::setupPose();
		}
	};
}

#endif
