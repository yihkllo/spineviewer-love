#include <spine/PointAttachment.h>

#include <spine/Bone.h>

#include <spine/MathUtil.h>

using namespace spine;

RTTI_IMPL(PointAttachment, Attachment)

PointAttachment::PointAttachment(const String &name) : Attachment(name), _x(0), _y(0), _rotation(0), _color(0.9451f, 0.9451f, 0, 1) {
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

void PointAttachment::computeWorldPosition(BonePose &bone, float &ox, float &oy) {
	ox = _x * bone.getA() + _y * bone.getB() + bone.getWorldX();
	oy = _x * bone.getC() + _y * bone.getD() + bone.getWorldY();
}

float PointAttachment::computeWorldRotation(BonePose &bone) {
	float r = _rotation * MathUtil::Deg_Rad, cosine = MathUtil::cos(r), sine = MathUtil::sin(r);
	float x = cosine * bone.getA() + sine * bone.getB();
	float y = cosine * bone.getC() + sine * bone.getD();
	return MathUtil::atan2Deg(y, x);
}

Attachment &PointAttachment::copy() {
	PointAttachment *copy = new (__FILE__, __LINE__) PointAttachment(getName());
	copy->setTimelineAttachment(getTimelineAttachment());
	copy->setTimelineSlots(getTimelineSlots());
	copy->_x = _x;
	copy->_y = _y;
	copy->_rotation = _rotation;
	copy->_color.set(_color);
	return *copy;
}
