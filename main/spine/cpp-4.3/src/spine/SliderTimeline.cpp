#include <spine/SliderTimeline.h>

#include <spine/Event.h>
#include <spine/Skeleton.h>

#include <spine/Animation.h>
#include <spine/Slider.h>
#include <spine/SliderData.h>
#include <spine/SliderPose.h>
#include <spine/Property.h>

using namespace spine;

RTTI_IMPL(SliderTimeline, ConstraintTimeline1)

SliderTimeline::SliderTimeline(size_t frameCount, size_t bezierCount, int sliderIndex)
	: ConstraintTimeline1(frameCount, bezierCount, sliderIndex, Property_SliderTime) {
	PropertyId ids[] = {((PropertyId) Property_SliderTime << 32) | sliderIndex};
	setPropertyIds(ids, 1);
}

SliderTimeline::~SliderTimeline() {
}

void SliderTimeline::apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add, bool out,
						   bool appliedPose) {
	SP_UNUSED(lastTime);
	SP_UNUSED(events);
	SP_UNUSED(out);

	Slider *constraint = (Slider *) skeleton._constraints[_constraintIndex];
	if (constraint->isActive()) {
		SliderPose &pose = appliedPose ? *constraint->_appliedPose : constraint->_pose;
		SliderData &data = constraint->_data;
		pose._time = getAbsoluteValue(time, alpha, from, add, pose._time, data._setupPose._time);
	}
}
