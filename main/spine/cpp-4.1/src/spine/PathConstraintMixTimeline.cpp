
#include <spine/PathConstraintMixTimeline.h>

#include <spine/Event.h>
#include <spine/Skeleton.h>

#include <spine/Animation.h>
#include <spine/PathConstraint.h>
#include <spine/PathConstraintData.h>
#include <spine/Property.h>
#include <spine/Slot.h>
#include <spine/SlotData.h>

using namespace spine;

RTTI_IMPL(PathConstraintMixTimeline, CurveTimeline)

PathConstraintMixTimeline::PathConstraintMixTimeline(size_t frameCount, size_t bezierCount, int pathConstraintIndex)
	: CurveTimeline(frameCount, PathConstraintMixTimeline::ENTRIES, bezierCount),
	  _pathConstraintIndex(pathConstraintIndex) {
	PropertyId ids[] = {((PropertyId) Property_PathConstraintMix << 32) | pathConstraintIndex};
	setPropertyIds(ids, 1);
}

void PathConstraintMixTimeline::apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha,
									  MixBlend blend, MixDirection direction) {
	SP_UNUSED(lastTime);
	SP_UNUSED(pEvents);
	SP_UNUSED(direction);

	PathConstraint *constraintP = skeleton._pathConstraints[_pathConstraintIndex];
	PathConstraint &constraint = *constraintP;
	if (!constraint.isActive()) return;

	if (time < _frames[0]) {
		switch (blend) {
			case MixBlend_Setup:
				constraint._mixRotate = constraint._data._mixRotate;
				constraint._mixX = constraint._data._mixX;
				constraint._mixY = constraint._data._mixY;
				return;
			case MixBlend_First:
				constraint._mixRotate += (constraint._data._mixRotate - constraint._mixRotate) * alpha;
				constraint._mixX += (constraint._data._mixX - constraint._mixX) * alpha;
				constraint._mixY += (constraint._data._mixY - constraint._mixY) * alpha;
			default: {
			}
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

	if (blend == MixBlend_Setup) {
		PathConstraintData data = constraint._data;
		constraint._mixRotate = data._mixRotate + (rotate - data._mixRotate) * alpha;
		constraint._mixX = data._mixX + (x - data._mixX) * alpha;
		constraint._mixY = data._mixY + (y - data._mixY) * alpha;
	} else {
		constraint._mixRotate += (rotate - constraint._mixRotate) * alpha;
		constraint._mixX += (x - constraint._mixX) * alpha;
		constraint._mixY += (y - constraint._mixY) * alpha;
	}
}

void PathConstraintMixTimeline::setFrame(int frame, float time, float mixRotate, float mixX, float mixY) {
	frame *= ENTRIES;
	_frames[frame] = time;
	_frames[frame + ROTATE] = mixRotate;
	_frames[frame + X] = mixX;
	_frames[frame + Y] = mixY;
}
