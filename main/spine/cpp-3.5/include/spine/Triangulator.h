
#ifndef SPINE_TRIANGULATOR_H
#define SPINE_TRIANGULATOR_H

#include <spine/Array.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spTriangulator {
	spArrayFloatArray* convexPolygons;
	spArrayShortArray* convexPolygonsIndices;

	spShortArray* indicesArray;
	spIntArray* isConcaveArray;
	spShortArray* triangles;

	spArrayFloatArray* polygonPool;
	spArrayShortArray* polygonIndicesPool;
} spTriangulator;

spTriangulator* spTriangulator_create();
spShortArray* spTriangulator_triangulate(spTriangulator* self, spFloatArray* verticesArray);
spArrayFloatArray* spTriangulator_decompose(spTriangulator* self, spFloatArray* verticesArray, spShortArray* triangles);
void spTriangulator_dispose(spTriangulator* self);


#ifdef __cplusplus
}
#endif

#endif
