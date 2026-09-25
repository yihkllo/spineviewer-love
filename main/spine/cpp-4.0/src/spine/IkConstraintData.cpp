
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/IkConstraintData.h>

#include <spine/BoneData.h>

using namespace spine;

IkConstraintData::IkConstraintData(const String &name) : ConstraintData(name),
														 _target(NULL),
														 _bendDirection(1),
														 _compress(false),
														 _stretch(false),
														 _uniform(false),
														 _mix(1),
														 _softness(0) {
}

Vector<BoneData *> &IkConstraintData::getBones() {
	return _bones;
}

BoneData *IkConstraintData::getTarget() {
	return _target;
}

void IkConstraintData::setTarget(BoneData *inValue) {
	_target = inValue;
}

int IkConstraintData::getBendDirection() {
	return _bendDirection;
}

void IkConstraintData::setBendDirection(int inValue) {
	_bendDirection = inValue;
}

float IkConstraintData::getMix() {
	return _mix;
}

void IkConstraintData::setMix(float inValue) {
	_mix = inValue;
}

bool IkConstraintData::getStretch() {
	return _stretch;
}

void IkConstraintData::setStretch(bool inValue) {
	_stretch = inValue;
}

bool IkConstraintData::getCompress() {
	return _compress;
}

void IkConstraintData::setCompress(bool inValue) {
	_compress = inValue;
}


bool IkConstraintData::getUniform() {
	return _uniform;
}

void IkConstraintData::setUniform(bool inValue) {
	_uniform = inValue;
}

float IkConstraintData::getSoftness() {
	return _softness;
}

void IkConstraintData::setSoftness(float inValue) {
	_softness = inValue;
}
