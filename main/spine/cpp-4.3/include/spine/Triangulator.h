#ifndef Spine_Triangulator_h
#define Spine_Triangulator_h

#include <spine/Array.h>
#include <spine/Pool.h>

namespace spine {
	class SP_API Triangulator : public SpineObject {
	public:
		~Triangulator();

		Array<int> &triangulate(Array<float> &vertices);

		Array<Array<float> *> &decompose(Array<float> &vertices, Array<int> &triangles);

	private:
		Array<Array<float> *> _convexPolygons;
		Array<Array<int> *> _convexPolygonsIndices;

		Array<int> _indices;
		Array<bool> _isConcaveArray;
		Array<int> _triangles;

		Pool<Array<float>> _polygonPool;
		Pool<Array<int>> _polygonIndicesPool;

		static bool isConcave(int index, int vertexCount, const float *vertices, const int *indices);

		static bool positiveArea(float p1x, float p1y, float p2x, float p2y, float p3x, float p3y);

		static int winding(float p1x, float p1y, float p2x, float p2y, float p3x, float p3y);
	};
}

#endif
