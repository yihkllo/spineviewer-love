
#include <spine/PointAttachment.h>

#include <spine/Bone.h>

#include <spine/MathUtil.h>

using namespace spine;

RTTI_IMPL(PointAttachment, Attachment)

PointAttachment::PointAttachment(const String &name) : Attachment(name), _x(0), _y(0), _rotation(0), _color() {
}

void PointAttachment::computeWorldPosition(Bone &bone, float &ox, float &oy) {
	bone.localToWorld(_x, _y, ox, oy);
}

float PointAttachment::computeWorldRotation(Bone &bone) {
	float r = _rotation * MathUtil::Deg_Rad, cosine = MathUtil::cos(r), sine = MathUtil::sin(r);
	float x = cosine * bone._a + sine * bone._b;
	float y = cosine * bone._c + sine * bone._d;
	return MathUtil::atan2Deg(y, x);
}

float PointAttachment::getX() {
	return _x;
}

void PointAttachment::setX(float inValue) {
	_x = inValue;
}

float PointAttachment::getY() {
	return _y;
}

void PointAttachment::setY(float inValue) {
	_y = inValue;
}

float PointAttachment::getRotation() {
	return _rotation;
}

void PointAttachment::setRotation(float inValue) {
	_rotation = inValue;
}

Color &PointAttachment::getColor() {
	return _color;
}

Attachment *PointAttachment::copy() {
	PointAttachment *copy = new (__FILE__, __LINE__) PointAttachment(getName());
	copy->_x = _x;
	copy->_y = _y;
	copy->_rotation = _rotation;
	return copy;
}
