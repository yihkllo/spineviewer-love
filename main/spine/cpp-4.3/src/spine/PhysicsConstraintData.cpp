#include <spine/PhysicsConstraintData.h>
#include <spine/PhysicsConstraint.h>
#include <spine/BoneData.h>
#include <spine/Skeleton.h>

using namespace spine;

RTTI_IMPL(PhysicsConstraintData, ConstraintData)

PhysicsConstraintData::PhysicsConstraintData(const String &name)
	: ConstraintDataGeneric<PhysicsConstraint, PhysicsConstraintPose>(name), _bone(NULL), _x(0), _y(0), _rotate(0), _scaleX(0), _shearX(0), _limit(0),
	  _step(0), _scaleYMode(ScaleYMode_None), _inertiaGlobal(false), _strengthGlobal(false), _dampingGlobal(false), _massGlobal(false),
	  _windGlobal(false), _gravityGlobal(false), _mixGlobal(false) {
}

BoneData &PhysicsConstraintData::getBone() {
	return *_bone;
}

void PhysicsConstraintData::setBone(BoneData &bone) {
	_bone = &bone;
}

float PhysicsConstraintData::getStep() {
	return _step;
}

void PhysicsConstraintData::setStep(float step) {
	_step = step;
}

float PhysicsConstraintData::getX() {
	return _x;
}

void PhysicsConstraintData::setX(float x) {
	_x = x;
}

float PhysicsConstraintData::getY() {
	return _y;
}

void PhysicsConstraintData::setY(float y) {
	_y = y;
}

float PhysicsConstraintData::getRotate() {
	return _rotate;
}

void PhysicsConstraintData::setRotate(float rotate) {
	_rotate = rotate;
}

float PhysicsConstraintData::getScaleX() {
	return _scaleX;
}

void PhysicsConstraintData::setScaleX(float scaleX) {
	_scaleX = scaleX;
}

float PhysicsConstraintData::getShearX() {
	return _shearX;
}

void PhysicsConstraintData::setShearX(float shearX) {
	_shearX = shearX;
}

float PhysicsConstraintData::getLimit() {
	return _limit;
}

void PhysicsConstraintData::setLimit(float limit) {
	_limit = limit;
}

ScaleYMode PhysicsConstraintData::getScaleYMode() {
	return _scaleYMode;
}

void PhysicsConstraintData::setScaleYMode(ScaleYMode scaleYMode) {
	_scaleYMode = scaleYMode;
}

bool PhysicsConstraintData::getInertiaGlobal() {
	return _inertiaGlobal;
}

void PhysicsConstraintData::setInertiaGlobal(bool inertiaGlobal) {
	_inertiaGlobal = inertiaGlobal;
}

bool PhysicsConstraintData::getStrengthGlobal() {
	return _strengthGlobal;
}

void PhysicsConstraintData::setStrengthGlobal(bool strengthGlobal) {
	_strengthGlobal = strengthGlobal;
}

bool PhysicsConstraintData::getDampingGlobal() {
	return _dampingGlobal;
}

void PhysicsConstraintData::setDampingGlobal(bool dampingGlobal) {
	_dampingGlobal = dampingGlobal;
}

bool PhysicsConstraintData::getMassGlobal() {
	return _massGlobal;
}

void PhysicsConstraintData::setMassGlobal(bool massGlobal) {
	_massGlobal = massGlobal;
}

bool PhysicsConstraintData::getWindGlobal() {
	return _windGlobal;
}

void PhysicsConstraintData::setWindGlobal(bool windGlobal) {
	_windGlobal = windGlobal;
}

bool PhysicsConstraintData::getGravityGlobal() {
	return _gravityGlobal;
}

void PhysicsConstraintData::setGravityGlobal(bool gravityGlobal) {
	_gravityGlobal = gravityGlobal;
}

bool PhysicsConstraintData::getMixGlobal() {
	return _mixGlobal;
}

void PhysicsConstraintData::setMixGlobal(bool mixGlobal) {
	_mixGlobal = mixGlobal;
}

Constraint &PhysicsConstraintData::create(Skeleton &skeleton) {
	return *(new (__FILE__, __LINE__) PhysicsConstraint(*this, skeleton));
}
