#ifndef Spine_ConstraintTimeline_h
#define Spine_ConstraintTimeline_h

#include <spine/dll.h>
#include <spine/RTTI.h>

namespace spine {

	class SP_API ConstraintTimeline {
		RTTI_DECL_NOPARENT

	public:
		ConstraintTimeline();
		virtual ~ConstraintTimeline();

		virtual int getConstraintIndex() const = 0;

		virtual void setConstraintIndex(int inValue) = 0;
	};
}

#endif
