#include <spine/SpineObject.h>
#include <spine/Extension.h>

using namespace spine;

void *SpineObject::operator new(size_t sz) {
	return SpineExtension::getInstance()->_calloc(sz, __FILE__, __LINE__);
}

void *SpineObject::operator new(size_t sz, const char *file, int line) {
	return SpineExtension::getInstance()->_calloc(sz, file, line);
}

void *SpineObject::operator new(size_t sz, void *ptr) {
	SP_UNUSED(sz);
	return ptr;
}

void SpineObject::operator delete(void *p, const char *file, int line) {
	SpineExtension::free(p, file, line);
}

void SpineObject::operator delete(void *p, void *mem) {
	SP_UNUSED(mem);
	SpineExtension::free(p, __FILE__, __LINE__);
}

void SpineObject::operator delete(void *p) {
	SpineExtension::free(p, __FILE__, __LINE__);
}

SpineObject::~SpineObject() {
	SpineExtension::beforeFree(this);
}
