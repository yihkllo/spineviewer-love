
#include <spine/ClippingAttachment.h>

#include <spine/SlotData.h>

using namespace spine;

RTTI_IMPL(ClippingAttachment, VertexAttachment)

ClippingAttachment::ClippingAttachment(const String &name) : VertexAttachment(name), _endSlot(NULL), _color() {
}

SlotData *ClippingAttachment::getEndSlot() {
	return _endSlot;
}

void ClippingAttachment::setEndSlot(SlotData *inValue) {
	_endSlot = inValue;
}

Color &ClippingAttachment::getColor() {
	return _color;
}

Attachment *ClippingAttachment::copy() {
	ClippingAttachment *copy = new (__FILE__, __LINE__) ClippingAttachment(getName());
	copyTo(copy);
	copy->_endSlot = _endSlot;
	return copy;
}
