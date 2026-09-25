
#include <spine/IkConstraintTimeline.h>

#include <spine/Event.h>
#include <spine/Skeleton.h>

#include <spine/Animation.h>
#include <spine/IkConstraint.h>
#include <spine/IkConstraintData.h>
#include <spine/Property.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>

using namespace spine;

RTTI_IMPL(IkConstraintTimeline, CurveTimeline)

IkConstraintTimeline::IkConstraintTimeline(size_t frameCount, size_t bezierCount, int ikConstraintIndex)
	: CurveTimeline(frameCount, IkConstraintTimeline::ENTRIES, bezierCount), _ikConstraintIndex(ikConstraintIndex) {
	PropertyId ids[] = {((PropertyId) Property_IkConstraint << 32) | ikConstraintIndex};
	setPropertyIds(ids, 1);
}

void IkConstraintTimeline::apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha,
								 MixBlend blend, MixDirection direction) {
	SP_UNUSED(lastTime);
	SP_UNUSED(pEvents);

	IkConstraint *constraintP = skeleton._ikConstraints[_ikConstraintIndex];
	IkConstraint &constraint = *constraintP;
	if (!constraint.isActive()) return;

	if (time < _frames[0]) {
		switch (blend) {
			case MixBlend_Setup:
				constraint._mix = constraint._data._mix;
				constraint._softness = constraint._data._softness;
				constraint._bendDirection = constraint._data._bendDirection;
				constraint._compress = constraint._data._compress;
				constraint._stretch = constraint._data._stretch;
				return;
			case MixBlend_First:
				constraint._mix += (constraint._data._mix - constraint._mix) * alpha;
				constraint._softness += (constraint._data._softness - constraint._softness) * alpha;
				constraint._bendDirection = constraint._data._bendDirection;
				constraint._compress = constraint._data._compress;
				constraint._stretch = constraint._data._stretch;
				return;
			default:
				return;
		}
	}

	float mix = 0, softness = 0;
	int i = Animation::search(_frames, time, IkConstraintTimeline::ENTRIES);
	int curveType = (int) _curves[i / IkConstraintTimeline::ENTRIES];
	switch (curveType) {
		case IkConstraintTimeline::LINEAR: {
			float before = _frames[i];
			mix = _frames[i + IkConstraintTimeline::MIX];
			softness = _frames[i + IkConstraintTimeline::SOFTNESS];
			float t = (time - before) / (_frames[i + IkConstraintTimeline::ENTRIES] - before);
			mix += (_frames[i + IkConstraintTimeline::ENTRIES + IkConstraintTimeline::MIX] - mix) * t;
			softness += (_frames[i + IkConstraintTimeline::ENTRIES + IkConstraintTimeline::SOFTNESS] - softness) * t;
			break;
		}
		case IkConstraintTimeline::STEPPED: {
			mix = _frames[i + IkConstraintTimeline::MIX];
			softness = _frames[i + IkConstraintTimeline::SOFTNESS];
			break;
		}
		default: {
			mix = getBezierValue(time, i, IkConstraintTimeline::MIX, curveType - IkConstraintTimeline::BEZIER);
			softness = getBezierValue(time, i, IkConstraintTimeline::SOFTNESS,
									  curveType + IkConstraintTimeline::BEZIER_SIZE -
											  IkConstraintTimeline::BEZIER);
		}
	}

	if (blend == MixBlend_Setup) {
		constraint._mix = constraint._data._mix + (mix - constraint._data._mix) * alpha;
		constraint._softness = constraint._data._softness + (softness - constraint._data._softness) * alpha;

		if (direction == MixDirection_Out) {
			constraint._bendDirection = constraint._data._bendDirection;
			constraint._compress = constraint._data._compress;
			constraint._stretch = constraint._data._stretch;
		} else {
			constraint._bendDirection = _frames[i + IkConstraintTimeline::BEND_DIRECTION];
			constraint._compress = _frames[i + IkConstraintTimeline::COMPRESS] != 0;
			constraint._stretch = _frames[i + IkConstraintTimeline::STRETCH] != 0;
		}
	} else {
		constraint._mix += (mix - constraint._mix) * alpha;
		constraint._softness += (softness - constraint._softness) * alpha;
		if (direction == MixDirection_In) {
			constraint._bendDirection = _frames[i + IkConstraintTimeline::BEND_DIRECTION];
			constraint._compress = _frames[i + IkConstraintTimeline::COMPRESS] != 0;
			constraint._stretch = _frames[i + IkConstraintTimeline::STRETCH] != 0;
		}
	}
}

void IkConstraintTimeline::setFrame(int frame, float time, float mix, float softness, int bendDirection, bool compress,
									bool stretch) {
	frame *= ENTRIES;
	_frames[frame] = time;
	_frames[frame + MIX] = mix;
	_frames[frame + SOFTNESS] = softness;
	_frames[frame + BEND_DIRECTION] = (float) bendDirection;
	_frames[frame + COMPRESS] = compress ? 1 : 0;
	_frames[frame + STRETCH] = stretch ? 1 : 0;
}
