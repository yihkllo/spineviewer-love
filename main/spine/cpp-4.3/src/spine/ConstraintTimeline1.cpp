#include <spine/ConstraintTimeline1.h>

using namespace spine;

RTTI_IMPL_MULTI(ConstraintTimeline1, CurveTimeline1, ConstraintTimeline)

ConstraintTimeline1::ConstraintTimeline1(size_t frameCount, size_t bezierCount, int constraintIndex, Property property)
	: CurveTimeline1(frameCount, bezierCount), ConstraintTimeline(), _constraintIndex(constraintIndex) {
	PropertyId ids[] = {((PropertyId) property << 32) | constraintIndex};
	setPropertyIds(ids, 1);
}

ConstraintTimeline1::~ConstraintTimeline1() {
}
