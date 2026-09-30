#include <spine/PhysicsConstraintPose.h>

using namespace spine;

PhysicsConstraintPose::PhysicsConstraintPose()
	: Pose<PhysicsConstraintPose>(), _inertia(0), _strength(0), _damping(0), _massInverse(0), _wind(0), _gravity(0), _mix(0) {
}

PhysicsConstraintPose::~PhysicsConstraintPose() {
}

void PhysicsConstraintPose::set(PhysicsConstraintPose &pose) {
	_inertia = pose._inertia;
	_strength = pose._strength;
	_damping = pose._damping;
	_massInverse = pose._massInverse;
	_wind = pose._wind;
	_gravity = pose._gravity;
	_mix = pose._mix;
}

float PhysicsConstraintPose::getInertia() {
	return _inertia;
}

void PhysicsConstraintPose::setInertia(float inertia) {
	_inertia = inertia;
}

float PhysicsConstraintPose::getStrength() {
	return _strength;
}

void PhysicsConstraintPose::setStrength(float strength) {
	_strength = strength;
}

float PhysicsConstraintPose::getDamping() {
	return _damping;
}

void PhysicsConstraintPose::setDamping(float damping) {
	_damping = damping;
}

float PhysicsConstraintPose::getMassInverse() {
	return _massInverse;
}

void PhysicsConstraintPose::setMassInverse(float massInverse) {
	_massInverse = massInverse;
}

float PhysicsConstraintPose::getWind() {
	return _wind;
}

void PhysicsConstraintPose::setWind(float wind) {
	_wind = wind;
}

float PhysicsConstraintPose::getGravity() {
	return _gravity;
}

void PhysicsConstraintPose::setGravity(float gravity) {
	_gravity = gravity;
}

float PhysicsConstraintPose::getMix() {
	return _mix;
}

void PhysicsConstraintPose::setMix(float mix) {
	_mix = mix;
}
