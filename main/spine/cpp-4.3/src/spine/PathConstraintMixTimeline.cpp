#include <spine/PathConstraintMixTimeline.h>

#include <spine/Event.h>
#include <spine/Skeleton.h>

#include <spine/Animation.h>
#include <spine/PathConstraint.h>
#include <spine/PathConstraintData.h>
#include <spine/PathConstraintPose.h>
#include <spine/Property.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>

using namespace spine;

RTTI_IMPL_MULTI(PathConstraintMixTimeline, CurveTimeline, ConstraintTimeline)

PathConstraintMixTimeline::PathConstraintMixTimeline(size_t frameCount, size_t bezierCount, int constraintIndex)
	: CurveTimeline(frameCount, PathConstraintMixTimeline::ENTRIES, bezierCount), ConstraintTimeline(), _constraintIndex(constraintIndex) {
	PropertyId ids[] = {((PropertyId) Property_PathConstraintMix << 32) | constraintIndex};
	setPropertyIds(ids, 1);
}

PathConstraintMixTimeline::~PathConstraintMixTimeline() {
}

void PathConstraintMixTimeline::apply(Skeleton &skeleton, float lastTime, float time, Array<Event *> *events, float alpha, MixFrom from, bool add,
									  bool out, bool appliedPose) {
	SP_UNUSED(lastTime);
	SP_UNUSED(events);
	SP_UNUSED(out);

	PathConstraint *constraint = (PathConstraint *) skeleton._constraints[_constraintIndex];
	if (!constraint->isActive()) return;
	PathConstraintPose &pose = appliedPose ? *constraint->_appliedPose : constraint->_pose;

	if (time < _frames[0]) {
		PathConstraintPose &setup = constraint->_data._setupPose;
		switch (from) {
			case MixFrom_Setup:
				pose._mixRotate = setup._mixRotate;
				pose._mixX = setup._mixX;
				pose._mixY = setup._mixY;
				break;
			case MixFrom_First:
				pose._mixRotate += (setup._mixRotate - pose._mixRotate) * alpha;
				pose._mixX += (setup._mixX - pose._mixX) * alpha;
				pose._mixY += (setup._mixY - pose._mixY) * alpha;
				break;
			case MixFrom_Current:
				break;
		}
		return;
	}

	float rotate, x, y;
	int i = Animation::search(_frames, time, PathConstraintMixTimeline::ENTRIES);
	int curveType = (int) _curves[i >> 2];
	switch (curveType) {
		case LINEAR: {
			float before = _frames[i];
			rotate = _frames[i + ROTATE];
			x = _frames[i + X];
			y = _frames[i + Y];
			float t = (time - before) / (_frames[i + ENTRIES] - before);
			rotate += (_frames[i + ENTRIES + ROTATE] - rotate) * t;
			x += (_frames[i + ENTRIES + X] - x) * t;
			y += (_frames[i + ENTRIES + Y] - y) * t;
			break;
		}
		case STEPPED: {
			rotate = _frames[i + ROTATE];
			x = _frames[i + X];
			y = _frames[i + Y];
			break;
		}
		default: {
			rotate = getBezierValue(time, i, ROTATE, curveType - BEZIER);
			x = getBezierValue(time, i, X, curveType + BEZIER_SIZE - BEZIER);
			y = getBezierValue(time, i, Y, curveType + BEZIER_SIZE * 2 - BEZIER);
		}
	}

	PathConstraintPose &base = from == MixFrom_Setup ? constraint->_data._setupPose : pose;
	if (add) {
		pose._mixRotate = base._mixRotate + rotate * alpha;
		pose._mixX = base._mixX + x * alpha;
		pose._mixY = base._mixY + y * alpha;
	} else {
		pose._mixRotate = base._mixRotate + (rotate - base._mixRotate) * alpha;
		pose._mixX = base._mixX + (x - base._mixX) * alpha;
		pose._mixY = base._mixY + (y - base._mixY) * alpha;
	}
}

void PathConstraintMixTimeline::setFrame(int frame, float time, float mixRotate, float mixX, float mixY) {
	frame *= ENTRIES;
	_frames[frame] = time;
	_frames[frame + ROTATE] = mixRotate;
	_frames[frame + X] = mixX;
	_frames[frame + Y] = mixY;
}
