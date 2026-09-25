
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/ScaleTimeline.h>

#include <spine/Skeleton.h>
#include <spine/Event.h>

#include <spine/Slot.h>
#include <spine/SlotData.h>
#include <spine/Bone.h>
#include <spine/BoneData.h>

using namespace spine;

RTTI_IMPL(ScaleTimeline, TranslateTimeline)

ScaleTimeline::ScaleTimeline(int frameCount) : TranslateTimeline(frameCount) {
}

void ScaleTimeline::apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha,
	MixBlend blend, MixDirection direction
) {
	SP_UNUSED(lastTime);
	SP_UNUSED(pEvents);

	Bone *boneP = skeleton._bones[_boneIndex];
	Bone &bone = *boneP;

	if (!bone._active) return;

	if (time < _frames[0]) {
		switch (blend) {
		case MixBlend_Setup:
			bone._scaleX = bone._data._scaleX;
			bone._scaleY = bone._data._scaleY;
			return;
		case MixBlend_First:
			bone._scaleX += (bone._data._scaleX - bone._scaleX) * alpha;
			bone._scaleY += (bone._data._scaleY - bone._scaleY) * alpha;
		default: {}
		}
		return;
	}

	float x, y;
	if (time >= _frames[_frames.size() - ENTRIES]) {
		x = _frames[_frames.size() + PREV_X] * bone._data._scaleX;
		y = _frames[_frames.size() + PREV_Y] * bone._data._scaleY;
	} else {
		int frame = Animation::binarySearch(_frames, time, ENTRIES);
		x = _frames[frame + PREV_X];
		y = _frames[frame + PREV_Y];
		float frameTime = _frames[frame];
		float percent = getCurvePercent(frame / ENTRIES - 1,
			1 - (time - frameTime) / (_frames[frame + PREV_TIME] - frameTime));

		x = (x + (_frames[frame + X] - x) * percent) * bone._data._scaleX;
		y = (y + (_frames[frame + Y] - y) * percent) * bone._data._scaleY;
	}

	if (alpha == 1) {
		if (blend == MixBlend_Add) {
			bone._scaleX += x - bone._data._scaleX;
			bone._scaleY += y - bone._data._scaleY;
		} else {
			bone._scaleX = x;
			bone._scaleY = y;
		}
	} else {
		float bx, by;
		if (direction == MixDirection_Out) {
			switch (blend) {
			case MixBlend_Setup:
				bx = bone._data._scaleX;
				by = bone._data._scaleY;
				bone._scaleX = bx + (MathUtil::abs(x) * MathUtil::sign(bx) - bx) * alpha;
				bone._scaleY = by + (MathUtil::abs(y) * MathUtil::sign(by) - by) * alpha;
				break;
			case MixBlend_First:
			case MixBlend_Replace:
				bx = bone._scaleX;
				by = bone._scaleY;
				bone._scaleX = bx + (MathUtil::abs(x) * MathUtil::sign(bx) - bx) * alpha;
				bone._scaleY = by + (MathUtil::abs(y) * MathUtil::sign(by) - by) * alpha;
				break;
			case MixBlend_Add:
				bx = bone._scaleX;
				by = bone._scaleY;
				bone._scaleX = bx + (MathUtil::abs(x) * MathUtil::sign(bx) - bone._data._scaleX) * alpha;
				bone._scaleY = by + (MathUtil::abs(y) * MathUtil::sign(by) - bone._data._scaleY) * alpha;
			}
		} else {
			switch (blend) {
			case MixBlend_Setup:
				bx = MathUtil::abs(bone._data._scaleX) * MathUtil::sign(x);
				by = MathUtil::abs(bone._data._scaleY) * MathUtil::sign(y);
				bone._scaleX = bx + (x - bx) * alpha;
				bone._scaleY = by + (y - by) * alpha;
				break;
			case MixBlend_First:
			case MixBlend_Replace:
				bx = MathUtil::abs(bone._scaleX) * MathUtil::sign(x);
				by = MathUtil::abs(bone._scaleY) * MathUtil::sign(y);
				bone._scaleX = bx + (x - bx) * alpha;
				bone._scaleY = by + (y - by) * alpha;
				break;
			case MixBlend_Add:
				bx = MathUtil::sign(x);
				by = MathUtil::sign(y);
				bone._scaleX = MathUtil::abs(bone._scaleX) * bx + (x - MathUtil::abs(bone._data._scaleX) * bx) * alpha;
				bone._scaleY = MathUtil::abs(bone._scaleY) * by + (y - MathUtil::abs(bone._data._scaleY) * by) * alpha;
			}
		}
	}
}

int ScaleTimeline::getPropertyId() {
	return ((int) TimelineType_Scale << 24) + _boneIndex;
}
