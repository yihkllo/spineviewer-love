#include <spine/IkConstraintTimeline.h>

#include <spine/Event.h>
#include <spine/Skeleton.h>

#include <spine/Animation.h>
#include <spine/IkConstraint.h>
#include <spine/IkConstraintData.h>
#include <spine/IkConstraintPose.h>
#include <spine/Property.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>

using namespace spine;

RTTI_IMPL_MULTI(IkConstraintTimeline, CurveTimeline, ConstraintTimeline)

IkConstraintTimeline::IkConstraintTimeline(size_t frameCount, size_t bezierCount, int constraintIndex)
	: CurveTimeline(frameCount, IkConstraintTimeline::ENTRIES, bezierCount), ConstraintTimeline(), _constraintIndex(constraintIndex) {
	PropertyId ids[] = {((PropertyId) Property_IkConstraint << 32) | constraintIndex};
	setPropertyIds(ids, 1);
}

IkConstraintTimeline::~IkConstraintTimeline() {
}

void IkConstraintTimeline::apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add,
								 bool out, bool appliedPose) {
	SP_UNUSED(lastTime);
	SP_UNUSED(events);
	SP_UNUSED(add);

	IkConstraint *constraint = (IkConstraint *) skeleton._constraints[_constraintIndex];
	if (!constraint->isActive()) return;
	IkConstraintPose &pose = appliedPose ? *constraint->_appliedPose : constraint->_pose;

	if (time < _frames[0]) {
		IkConstraintPose &setup = constraint->_data._setupPose;
		switch (from) {
			case MixFrom_Setup:
				pose._mix = setup._mix;
				pose._softness = setup._softness;
				pose._bendDirection = setup._bendDirection;
				pose._compress = setup._compress;
				pose._stretch = setup._stretch;
				break;
			case MixFrom_First:
				pose._mix += (setup._mix - pose._mix) * alpha;
				pose._softness += (setup._softness - pose._softness) * alpha;
				pose._bendDirection = setup._bendDirection;
				pose._compress = setup._compress;
				pose._stretch = setup._stretch;
				break;
			case MixFrom_Current:
				break;
		}
		return;
	}

	float mix = 0, softness = 0;
	int i = Animation::search(_frames, time, ENTRIES);
	int curveType = (int) _curves[i / ENTRIES];
	switch (curveType) {
		case LINEAR: {
			float before = _frames[i];
			mix = _frames[i + MIX];
			softness = _frames[i + SOFTNESS];
			float t = (time - before) / (_frames[i + ENTRIES] - before);
			mix += (_frames[i + ENTRIES + MIX] - mix) * t;
			softness += (_frames[i + ENTRIES + SOFTNESS] - softness) * t;
			break;
		}
		case STEPPED: {
			mix = _frames[i + MIX];
			softness = _frames[i + SOFTNESS];
			break;
		}
		default: {
			mix = getBezierValue(time, i, MIX, curveType - BEZIER);
			softness = getBezierValue(time, i, SOFTNESS, curveType + BEZIER_SIZE - BEZIER);
		}
	}

	IkConstraintPose &base = from == MixFrom_Setup ? constraint->_data._setupPose : pose;
	pose._mix = base._mix + (mix - base._mix) * alpha;
	pose._softness = base._softness + (softness - base._softness) * alpha;
	if (out) {
		if (from == MixFrom_Setup) {
			pose._bendDirection = base._bendDirection;
			pose._compress = base._compress;
			pose._stretch = base._stretch;
		}
	} else {
		pose._bendDirection = (int) _frames[i + BEND_DIRECTION];
		pose._compress = _frames[i + COMPRESS] != 0;
		pose._stretch = _frames[i + STRETCH] != 0;
	}
}

void IkConstraintTimeline::setFrame(int frame, float time, float mix, float softness, int bendDirection, bool compress, bool stretch) {
	frame *= ENTRIES;
	_frames[frame] = time;
	_frames[frame + MIX] = mix;
	_frames[frame + SOFTNESS] = softness;
	_frames[frame + BEND_DIRECTION] = (float) bendDirection;
	_frames[frame + COMPRESS] = compress ? 1 : 0;
	_frames[frame + STRETCH] = stretch ? 1 : 0;
}
