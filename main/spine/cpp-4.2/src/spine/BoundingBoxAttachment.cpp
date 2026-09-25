
#include <spine/BoundingBoxAttachment.h>

using namespace spine;

RTTI_IMPL(BoundingBoxAttachment, VertexAttachment)

BoundingBoxAttachment::BoundingBoxAttachment(const String &name) : VertexAttachment(name), _color() {
}

Color &BoundingBoxAttachment::getColor() {
	return _color;
}

Attachment *BoundingBoxAttachment::copy() {
	BoundingBoxAttachment *copy = new (__FILE__, __LINE__) BoundingBoxAttachment(getName());
	copyTo(copy);
	return copy;
}
