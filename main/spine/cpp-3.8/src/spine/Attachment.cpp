
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/Attachment.h>

#include <assert.h>

using namespace spine;

RTTI_IMPL_NOPARENT(Attachment)

Attachment::Attachment(const String &name) : _name(name), _refCount(0) {
	assert(_name.length() > 0);
}

Attachment::~Attachment() {
}

const String &Attachment::getName() const {
	return _name;
}

int Attachment::getRefCount() {
	return _refCount;
}

void Attachment::reference() {
	_refCount++;
}

void Attachment::dereference() {
	_refCount--;
}
