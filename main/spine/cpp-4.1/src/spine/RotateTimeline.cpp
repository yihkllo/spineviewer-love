
#include <spine/RotateTimeline.h>

#include <spine/Event.h>
#include <spine/Skeleton.h>

#include <spine/Animation.h>
#include <spine/Bone.h>
#include <spine/BoneData.h>
#include <spine/Property.h>

using namespace spine;

RTTI_IMPL(RotateTimeline, CurveTimeline1)

RotateTimeline::RotateTimeline(size_t frameCount, size_t bezierCount, int boneIndex) : CurveTimeline1(frameCount,
																									  bezierCount),
																					   _boneIndex(boneIndex) {
	PropertyId ids[] = {((PropertyId) Property_Rotate << 32) | boneIndex};
	setPropertyIds(ids, 1);
}

void RotateTimeline::apply(Skeleton &skeleton, float lastTime, float time, Vector<Event *> *pEvents, float alpha,
						   MixBlend blend, MixDirection direction) {
	SP_UNUSED(lastTime);
	SP_UNUSED(pEvents);
	SP_UNUSED(direction);

	Bone *bone = skeleton._bones[_boneIndex];
	if (!bone->_active) return;

	if (time < _frames[0]) {
		switch (blend) {
			case MixBlend_Setup:
				bone->_rotation = bone->_data._rotation;
				return;
			case MixBlend_First:
				bone->_rotation += (bone->_data._rotation - bone->_rotation) * alpha;
			default: {
			}
		}
		return;
	}

	float r = getCurveValue(time);
	switch (blend) {
		case MixBlend_Setup:
			bone->_rotation = bone->_data._rotation + r * alpha;
			break;
		case MixBlend_First:
		case MixBlend_Replace:
			r += bone->_data._rotation - bone->_rotation;
		case MixBlend_Add:
			bone->_rotation += r * alpha;
	}
}
