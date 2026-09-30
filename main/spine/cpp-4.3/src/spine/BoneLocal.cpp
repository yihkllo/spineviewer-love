#include <spine/BoneLocal.h>

using namespace spine;

BoneLocal::BoneLocal() : _x(0), _y(0), _rotation(0), _scaleX(1), _scaleY(1), _shearX(0), _shearY(0), _inherit(Inherit_Normal) {
}

BoneLocal::~BoneLocal() {
}

void BoneLocal::set(BoneLocal &pose) {
	_x = pose._x;
	_y = pose._y;
	_rotation = pose._rotation;
	_scaleX = pose._scaleX;
	_scaleY = pose._scaleY;
	_shearX = pose._shearX;
	_shearY = pose._shearY;
	_inherit = pose._inherit;
}

float BoneLocal::getX() {
	return _x;
}

void BoneLocal::setX(float x) {
	_x = x;
}

float BoneLocal::getY() {
	return _y;
}

void BoneLocal::setY(float y) {
	_y = y;
}

void BoneLocal::setPosition(float x, float y) {
	_x = x;
	_y = y;
}

float BoneLocal::getRotation() {
	return _rotation;
}

void BoneLocal::setRotation(float rotation) {
	_rotation = rotation;
}

float BoneLocal::getScaleX() {
	return _scaleX;
}

void BoneLocal::setScaleX(float scaleX) {
	_scaleX = scaleX;
}

float BoneLocal::getScaleY() {
	return _scaleY;
}

void BoneLocal::setScaleY(float scaleY) {
	_scaleY = scaleY;
}

void BoneLocal::setScale(float scaleX, float scaleY) {
	_scaleX = scaleX;
	_scaleY = scaleY;
}

void BoneLocal::setScale(float scale) {
	_scaleX = scale;
	_scaleY = scale;
}

float BoneLocal::getShearX() {
	return _shearX;
}

void BoneLocal::setShearX(float shearX) {
	_shearX = shearX;
}

float BoneLocal::getShearY() {
	return _shearY;
}

void BoneLocal::setShearY(float shearY) {
	_shearY = shearY;
}

Inherit BoneLocal::getInherit() {
	return _inherit;
}

void BoneLocal::setInherit(Inherit inherit) {
	_inherit = inherit;
}
