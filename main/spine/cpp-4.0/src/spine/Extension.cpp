
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/Extension.h>
#include <spine/SpineString.h>

#include <assert.h>

using namespace spine;

SpineExtension *SpineExtension::_instance = NULL;

void SpineExtension::setInstance(SpineExtension *inValue) {
	assert(inValue);

	_instance = inValue;
}

SpineExtension *SpineExtension::getInstance() {
	if (!_instance) _instance = spine::getDefaultExtension();
	assert(_instance);

	return _instance;
}

SpineExtension::~SpineExtension() {
}

SpineExtension::SpineExtension() {
}

DefaultSpineExtension::~DefaultSpineExtension() {
}

void *DefaultSpineExtension::_alloc(size_t size, const char *file, int line) {
	SP_UNUSED(file);
	SP_UNUSED(line);

	if (size == 0)
		return 0;
	void *ptr = ::malloc(size);
	return ptr;
}

void *DefaultSpineExtension::_calloc(size_t size, const char *file, int line) {
	SP_UNUSED(file);
	SP_UNUSED(line);

	if (size == 0)
		return 0;

	void *ptr = ::malloc(size);
	if (ptr) {
		memset(ptr, 0, size);
	}
	return ptr;
}

void *DefaultSpineExtension::_realloc(void *ptr, size_t size, const char *file, int line) {
	SP_UNUSED(file);
	SP_UNUSED(line);

	void *mem = NULL;
	if (size == 0)
		return 0;
	if (ptr == NULL)
		mem = ::malloc(size);
	else
		mem = ::realloc(ptr, size);
	return mem;
}

void DefaultSpineExtension::_free(void *mem, const char *file, int line) {
	SP_UNUSED(file);
	SP_UNUSED(line);

	::free(mem);
}

char *DefaultSpineExtension::_readFile(const String &path, int *length) {
	char *data;
	FILE *file = fopen(path.buffer(), "rb");
	if (!file) return 0;

	fseek(file, 0, SEEK_END);
	*length = (int) ftell(file);
	fseek(file, 0, SEEK_SET);

	data = SpineExtension::alloc<char>(*length, __FILE__, __LINE__);
	fread(data, 1, *length, file);
	fclose(file);

	return data;
}

DefaultSpineExtension::DefaultSpineExtension() : SpineExtension() {
}
