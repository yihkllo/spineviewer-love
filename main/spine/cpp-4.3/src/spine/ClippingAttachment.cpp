#include <spine/ClippingAttachment.h>

#include <spine/SlotData.h>

using namespace spine;

RTTI_IMPL(ClippingAttachment, VertexAttachment)

ClippingAttachment::ClippingAttachment(const String &name) : VertexAttachment(name), _endSlot(NULL), _convex(false), _inverse(false), _color() {
}

SlotData *ClippingAttachment::getEndSlot() {
	return _endSlot;
}

void ClippingAttachment::setEndSlot(SlotData *inValue) {
	_endSlot = inValue;
}

bool ClippingAttachment::getConvex() {
	return _convex;
}

void ClippingAttachment::setConvex(bool convex) {
	_convex = convex;
}

bool ClippingAttachment::getInverse() {
	return _inverse;
}

void ClippingAttachment::setInverse(bool inverse) {
	_inverse = inverse;
}

Color &ClippingAttachment::getColor() {
	return _color;
}

Attachment &ClippingAttachment::copy() {
	ClippingAttachment *copy = new (__FILE__, __LINE__) ClippingAttachment(getName());
	copyTo(*copy);
	copy->_endSlot = _endSlot;
	copy->_convex = _convex;
	copy->_inverse = _inverse;
	return *copy;
}
