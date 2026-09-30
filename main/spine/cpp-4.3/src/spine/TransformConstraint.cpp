#include <spine/TransformConstraint.h>

#include <spine/Bone.h>
#include <spine/BonePose.h>
#include <spine/Skeleton.h>
#include <spine/TransformConstraintData.h>
#include <spine/MathUtil.h>

#include <spine/BoneData.h>

using namespace spine;

RTTI_IMPL(TransformConstraint, Constraint)

TransformConstraint::TransformConstraint(TransformConstraintData &data, Skeleton &skeleton) : TransformConstraintBase(data) {

	_bones.ensureCapacity(data.getBones().size());
	for (size_t i = 0; i < data.getBones().size(); i++) {
		BoneData *boneData = data.getBones()[i];
		_bones.add(&skeleton._bones[boneData->getIndex()]->_constrainedPose);
	}

	_source = skeleton._bones[data._source->getIndex()];
}

TransformConstraint &TransformConstraint::copy(Skeleton &skeleton) {
	TransformConstraint *copy = new (__FILE__, __LINE__) TransformConstraint(_data, skeleton);
	copy->_pose.set(_pose);
	return *copy;
}

void TransformConstraint::update(Skeleton &skeleton, Physics physics) {
	TransformConstraintPose &p = *_appliedPose;
	if (p._mixRotate == 0 && p._mixX == 0 && p._mixY == 0 && p._mixScaleX == 0 && p._mixScaleY == 0 && p._mixShearY == 0) return;

	TransformConstraintData &data = _data;
	bool localSource = data._localSource, localTarget = data._localTarget, additive = data._additive, clamp = data._clamp;
	float *offsets = data._offsets;
	BonePose &source = *_source->_appliedPose;
	if (localSource) {
		source.validateLocalTransform(skeleton);
	}
	FromProperty **fromItems = data._properties.buffer();
	size_t fn = data._properties.size();
	BonePose **bones = _bones.buffer();
	for (size_t i = 0, n = _bones.size(); i < n; i++) {
		BonePose *bone = bones[i];
		if (localTarget) {
			bone->modifyLocal(skeleton);
		} else {
			bone->modifyWorld(skeleton);
		}
		for (size_t f = 0; f < fn; f++) {
			FromProperty *from = fromItems[f];
			float value = from->value(skeleton, source, localSource, offsets) - from->_offset;
			Array<ToProperty *> &toProps = from->_to;
			ToProperty **toItems = toProps.buffer();
			for (size_t t = 0, tn = toProps.size(); t < tn; t++) {
				ToProperty *to = toItems[t];
				if (to->mix(p) != 0) {
					float clamped = to->_offset + value * to->_scale;
					if (clamp) {
						if (to->_offset < to->_max)
							clamped = MathUtil::clamp(clamped, to->_offset, to->_max);
						else
							clamped = MathUtil::clamp(clamped, to->_max, to->_offset);
					}
					to->apply(skeleton, p, *bone, clamped, localTarget, additive);
				}
			}
		}
	}
}

void TransformConstraint::sort(Skeleton &skeleton) {
	if (!_data._localSource) skeleton.sortBone(_source);
	BonePose **bones = _bones.buffer();
	size_t boneCount = _bones.size();
	bool worldTarget = !_data._localTarget;
	if (worldTarget) {
		for (size_t i = 0; i < boneCount; i++) skeleton.sortBone(bones[i]->_bone);
	}
	skeleton._updateCache.add(this);
	for (size_t i = 0; i < boneCount; i++) {
		Bone *bone = bones[i]->_bone;
		skeleton.sortReset(bone->_children);
		skeleton.constrained(*bone);
	}
	for (size_t i = 0; i < boneCount; i++) bones[i]->_bone->_sorted = worldTarget;
}

bool TransformConstraint::isSourceActive() {
	return _source->_active;
}

Array<BonePose *> &TransformConstraint::getBones() {
	return _bones;
}

Bone &TransformConstraint::getSource() {
	return *_source;
}

void TransformConstraint::setSource(Bone &source) {
	_source = &source;
}
