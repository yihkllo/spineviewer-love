
#include <spine/PhysicsConstraintData.h>

#include <spine/BoneData.h>

#include <assert.h>

using namespace spine;

RTTI_IMPL(PhysicsConstraintData, ConstraintData)

PhysicsConstraintData::PhysicsConstraintData(const String &name) : ConstraintData(name),
																   _bone(nullptr),
																   _x(0), _y(0), _rotate(0), _scaleX(0), _shearX(0), _limit(0),
																   _step(0), _inertia(0), _strength(0), _damping(0), _massInverse(0), _wind(0), _gravity(0), _mix(0),
																   _inertiaGlobal(false), _strengthGlobal(false), _dampingGlobal(false), _massGlobal(false),
																   _windGlobal(false), _gravityGlobal(false), _mixGlobal(false) {
}


void PhysicsConstraintData::setBone(BoneData *bone) {
	_bone = bone;
}

BoneData *PhysicsConstraintData::getBone() const {
	return _bone;
}

void PhysicsConstraintData::setX(float x) {
	_x = x;
}

float PhysicsConstraintData::getX() const {
	return _x;
}

void PhysicsConstraintData::setY(float y) {
	_y = y;
}

float PhysicsConstraintData::getY() const {
	return _y;
}

void PhysicsConstraintData::setRotate(float rotate) {
	_rotate = rotate;
}

float PhysicsConstraintData::getRotate() const {
	return _rotate;
}

void PhysicsConstraintData::setScaleX(float scaleX) {
	_scaleX = scaleX;
}

float PhysicsConstraintData::getScaleX() const {
	return _scaleX;
}

void PhysicsConstraintData::setShearX(float shearX) {
	_shearX = shearX;
}

float PhysicsConstraintData::getShearX() const {
	return _shearX;
}

void PhysicsConstraintData::setLimit(float limit) {
	_limit = limit;
}

float PhysicsConstraintData::getLimit() const {
	return _limit;
}

void PhysicsConstraintData::setStep(float step) {
	_step = step;
}

float PhysicsConstraintData::getStep() const {
	return _step;
}

void PhysicsConstraintData::setInertia(float inertia) {
	_inertia = inertia;
}

float PhysicsConstraintData::getInertia() const {
	return _inertia;
}

void PhysicsConstraintData::setStrength(float strength) {
	_strength = strength;
}

float PhysicsConstraintData::getStrength() const {
	return _strength;
}

void PhysicsConstraintData::setDamping(float damping) {
	_damping = damping;
}

float PhysicsConstraintData::getDamping() const {
	return _damping;
}

void PhysicsConstraintData::setMassInverse(float massInverse) {
	_massInverse = massInverse;
}

float PhysicsConstraintData::getMassInverse() const {
	return _massInverse;
}

void PhysicsConstraintData::setWind(float wind) {
	_wind = wind;
}

float PhysicsConstraintData::getWind() const {
	return _wind;
}

void PhysicsConstraintData::setGravity(float gravity) {
	_gravity = gravity;
}

float PhysicsConstraintData::getGravity() const {
	return _gravity;
}

void PhysicsConstraintData::setMix(float mix) {
	_mix = mix;
}

float PhysicsConstraintData::getMix() const {
	return _mix;
}

void PhysicsConstraintData::setInertiaGlobal(bool inertiaGlobal) {
	_inertiaGlobal = inertiaGlobal;
}

bool PhysicsConstraintData::isInertiaGlobal() const {
	return _inertiaGlobal;
}

void PhysicsConstraintData::setStrengthGlobal(bool strengthGlobal) {
	_strengthGlobal = strengthGlobal;
}

bool PhysicsConstraintData::isStrengthGlobal() const {
	return _strengthGlobal;
}

void PhysicsConstraintData::setDampingGlobal(bool dampingGlobal) {
	_dampingGlobal = dampingGlobal;
}

bool PhysicsConstraintData::isDampingGlobal() const {
	return _dampingGlobal;
}

void PhysicsConstraintData::setMassGlobal(bool massGlobal) {
	_massGlobal = massGlobal;
}

bool PhysicsConstraintData::isMassGlobal() const {
	return _massGlobal;
}

void PhysicsConstraintData::setWindGlobal(bool windGlobal) {
	_windGlobal = windGlobal;
}

bool PhysicsConstraintData::isWindGlobal() const {
	return _windGlobal;
}

void PhysicsConstraintData::setGravityGlobal(bool gravityGlobal) {
	_gravityGlobal = gravityGlobal;
}

bool PhysicsConstraintData::isGravityGlobal() const {
	return _gravityGlobal;
}

void PhysicsConstraintData::setMixGlobal(bool mixGlobal) {
	_mixGlobal = mixGlobal;
}

bool PhysicsConstraintData::isMixGlobal() const {
	return _mixGlobal;
}
