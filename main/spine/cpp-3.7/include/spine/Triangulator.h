
#ifndef SPINE_TRIANGULATOR_H
#define SPINE_TRIANGULATOR_H

#include <spine/dll.h>
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

SP_API spTriangulator* spTriangulator_create();
SP_API spShortArray* spTriangulator_triangulate(spTriangulator* self, spFloatArray* verticesArray);
SP_API spArrayFloatArray* spTriangulator_decompose(spTriangulator* self, spFloatArray* verticesArray, spShortArray* triangles);
SP_API void spTriangulator_dispose(spTriangulator* self);


#ifdef __cplusplus
}
#endif

#endif
