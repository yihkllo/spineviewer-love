
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/PathAttachment.h>

using namespace spine;

RTTI_IMPL(PathAttachment, VertexAttachment)

PathAttachment::PathAttachment(const String &name) : VertexAttachment(name), _closed(false), _constantSpeed(false) {
}

Vector<float> &PathAttachment::getLengths() {
	return _lengths;
}

bool PathAttachment::isClosed() {
	return _closed;
}

void PathAttachment::setClosed(bool inValue) {
	_closed = inValue;
}

bool PathAttachment::isConstantSpeed() {
	return _constantSpeed;
}

void PathAttachment::setConstantSpeed(bool inValue) {
	_constantSpeed = inValue;
}

Attachment* PathAttachment::copy() {
	PathAttachment* copy = new (__FILE__, __LINE__) PathAttachment(getName());
	copyTo(copy);
	copy->_lengths.clearAndAddAll(_lengths);
	copy->_closed = _closed;
	copy->_constantSpeed = _constantSpeed;
	return copy;
}
