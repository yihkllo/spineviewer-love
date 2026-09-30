#include <spine/PathAttachment.h>

using namespace spine;

RTTI_IMPL(PathAttachment, VertexAttachment)

PathAttachment::PathAttachment(const String &name) : VertexAttachment(name), _closed(false), _constantSpeed(false), _color() {
}

Array<float> &PathAttachment::getLengths() {
	return _lengths;
}

void PathAttachment::setLengths(Array<float> &inValue) {
	_lengths.clearAndAddAll(inValue);
}

bool PathAttachment::getClosed() {
	return _closed;
}

void PathAttachment::setClosed(bool inValue) {
	_closed = inValue;
}

bool PathAttachment::getConstantSpeed() {
	return _constantSpeed;
}

void PathAttachment::setConstantSpeed(bool inValue) {
	_constantSpeed = inValue;
}

Color &PathAttachment::getColor() {
	return _color;
}

Attachment &PathAttachment::copy() {
	PathAttachment *copy = new (__FILE__, __LINE__) PathAttachment(getName());
	copyTo(*copy);
	copy->_lengths.clearAndAddAll(_lengths);
	copy->_closed = _closed;
	copy->_constantSpeed = _constantSpeed;
	return *copy;
}
