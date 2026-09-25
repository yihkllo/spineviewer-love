
#include <spine/extension.h>
#include <stdio.h>

static void* (*mallocFunc) (size_t size) = malloc;
static void* (*debugMallocFunc) (size_t size, const char* file, int line) = NULL;
static void (*freeFunc) (void* ptr) = free;

void* _malloc (size_t size, const char* file, int line) {
	if(debugMallocFunc)
		return debugMallocFunc(size, file, line);

	return mallocFunc(size);
}
void* _calloc (size_t num, size_t size, const char* file, int line) {
	void* ptr = _malloc(num * size, file, line);
	if (ptr) memset(ptr, 0, num * size);
	return ptr;
}
void _free (void* ptr) {
	freeFunc(ptr);
}

void _setDebugMalloc(void* (*malloc) (size_t size, const char* file, int line)) {
	debugMallocFunc = malloc;
}

void _setMalloc (void* (*malloc) (size_t size)) {
	mallocFunc = malloc;
}
void _setFree (void (*free) (void* ptr)) {
	freeFunc = free;
}

char* _readFile (const char* path, int* length) {
	char *data;
	FILE *file = fopen(path, "rb");
	if (!file) return 0;

	fseek(file, 0, SEEK_END);
	*length = (int)ftell(file);
	fseek(file, 0, SEEK_SET);

	data = MALLOC(char, *length);
	fread(data, 1, *length, file);
	fclose(file);

	return data;
}
