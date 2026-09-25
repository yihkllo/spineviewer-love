
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/ShearTimeline.h>

#include <spine/Skeleton.h>
#include <spine/Event.h>

#include <spine/Slot.h>
#include <spine/SlotData.h>
#include <spine/Bone.h>
#include <spine/BoneData.h>

using namespace spine;

RTTI_IMPL(ShearTimeline, TranslateTimeline)

ShearTimeline::ShearTimeline(int frameCount) : TranslateTimeline(frameCount) {
}

void ShearTimeline::apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha,
	MixBlend blend, MixDirection direction
) {
	SP_UNUSED(lastTime);
	SP_UNUSED(pEvents);
	SP_UNUSED(direction);

	Bone *boneP = skeleton._bones[_boneIndex];
	Bone &bone = *boneP;
	if (!bone._active) return;

	if (time < _frames[0]) {
		switch (blend) {
		case MixBlend_Setup:
			bone._shearX = bone._data._shearX;
			bone._shearY = bone._data._shearY;
			return;
		case MixBlend_First:
			bone._shearX += (bone._data._shearX - bone._shearX) * alpha;
			bone._shearY += (bone._data._shearY - bone._shearY) * alpha;
		default: {}
		}
		return;
	}

	float x, y;
	if (time >= _frames[_frames.size() - ENTRIES]) {
		x = _frames[_frames.size() + PREV_X];
		y = _frames[_frames.size() + PREV_Y];
	} else {
		int frame = Animation::binarySearch(_frames, time, ENTRIES);
		x = _frames[frame + PREV_X];
		y = _frames[frame + PREV_Y];
		float frameTime = _frames[frame];
		float percent = getCurvePercent(frame / ENTRIES - 1,
			1 - (time - frameTime) / (_frames[frame + PREV_TIME] - frameTime));

		x = x + (_frames[frame + X] - x) * percent;
		y = y + (_frames[frame + Y] - y) * percent;
	}

	switch (blend) {
		case MixBlend_Setup:
			bone._shearX = bone._data._shearX + x * alpha;
			bone._shearY = bone._data._shearY + y * alpha;
			break;
		case MixBlend_First:
		case MixBlend_Replace:
			bone._shearX += (bone._data._shearX + x - bone._shearX) * alpha;
			bone._shearY += (bone._data._shearY + y - bone._shearY) * alpha;
			break;
		case MixBlend_Add:
			bone._shearX += x * alpha;
			bone._shearY += y * alpha;
	}
}

int ShearTimeline::getPropertyId() {
	return ((int) TimelineType_Shear << 24) + _boneIndex;
}
