#ifndef Spine_ConstraintTimeline1_h
#define Spine_ConstraintTimeline1_h

#include <spine/ConstraintTimeline.h>
#include <spine/CurveTimeline.h>
#include <spine/Property.h>

namespace spine {

	class SP_API ConstraintTimeline1 : public CurveTimeline1, public ConstraintTimeline {
		RTTI_DECL

	public:
		ConstraintTimeline1(size_t frameCount, size_t bezierCount, int constraintIndex, Property property);
		virtual ~ConstraintTimeline1();

		virtual int getConstraintIndex() const override {
			return _constraintIndex;
		}

		virtual void setConstraintIndex(int inValue) override {
			_constraintIndex = inValue;
		}

	protected:
		int _constraintIndex;
	};
}

#endif
