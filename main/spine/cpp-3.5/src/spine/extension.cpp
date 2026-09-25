
#include <spine/extension.h>
#include <stdio.h>

float _spInternalRandom () {
	return rand() / (float)RAND_MAX;
}

static void* (*mallocFunc) (size_t size) = malloc;
static void* (*reallocFunc) (void* ptr, size_t size) = realloc;
static void* (*debugMallocFunc) (size_t size, const char* file, int line) = NULL;
static void (*freeFunc) (void* ptr) = free;
static float (*randomFunc) () = _spInternalRandom;

void* _spMalloc (size_t size, const char* file, int line) {
	if(debugMallocFunc)
		return debugMallocFunc(size, file, line);

	return mallocFunc(size);
}
void* _spCalloc (size_t num, size_t size, const char* file, int line) {
	void* ptr = _spMalloc(num * size, file, line);
	if (ptr) memset(ptr, 0, num * size);
	return ptr;
}
void* _spRealloc(void* ptr, size_t size) {
	return reallocFunc(ptr, size);
}
void _spFree (void* ptr) {
	freeFunc(ptr);
}

float _spRandom () {
	return randomFunc();
}

void _spSetDebugMalloc(void* (*malloc) (size_t size, const char* file, int line)) {
	debugMallocFunc = malloc;
}

void _spSetMalloc (void* (*malloc) (size_t size)) {
	mallocFunc = malloc;
}

void _spSetRealloc (void* (*realloc) (void* ptr, size_t size)) {
	reallocFunc = realloc;
}

void _spSetFree (void (*free) (void* ptr)) {
	freeFunc = free;
}

void _spSetRandom (float (*random) ()) {
	randomFunc = random;
}

char* _spReadFile (const char* path, int* length) {
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

float _spMath_random(float min, float max) {
	return min + (max - min) * _spRandom();
}

float _spMath_randomTriangular(float min, float max) {
	return _spMath_randomTriangularWith(min, max, (min + max) * 0.5f);
}

float _spMath_randomTriangularWith(float min, float max, float mode) {
	float u = _spRandom();
	float d = max - min;
	if (u <= (mode - min) / d) return min + SQRT(u * d * (mode - min));
	return max - SQRT((1 - u) * d * (max - mode));
}

float _spMath_interpolate(float (*apply) (float a), float start, float end, float a) {
	return start + (end - start) * apply(a);
}

float _spMath_pow2_apply(float a) {
	if (a <= 0.5) return POW(a * 2, 2) / 2;
	return POW((a - 1) * 2, 2) / -2 + 1;
}

float _spMath_pow2out_apply(float a) {
	return POW(a - 1, 2) * -1 + 1;
}
