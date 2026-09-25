
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/Triangulator.h>

#include <spine/MathUtil.h>

using namespace spine;

Triangulator::~Triangulator() {
	ContainerUtil::cleanUpVectorOfPointers(_convexPolygons);
	ContainerUtil::cleanUpVectorOfPointers(_convexPolygonsIndices);
}

Vector<int> &Triangulator::triangulate(Vector<float> &vertices) {
	size_t vertexCount = vertices.size() >> 1;

	Vector<int> &indices = _indices;
	indices.clear();
	indices.ensureCapacity(vertexCount);
	indices.setSize(vertexCount, 0);
	for (size_t i = 0; i < vertexCount; ++i) {
		indices[i] = i;
	}

	Vector<bool> &isConcaveArray = _isConcaveArray;
	isConcaveArray.ensureCapacity(vertexCount);
	isConcaveArray.setSize(vertexCount, 0);
	for (size_t i = 0, n = vertexCount; i < n; ++i) {
		isConcaveArray[i] = isConcave(i, vertexCount, vertices, indices);
	}

	Vector<int> &triangles = _triangles;
	triangles.clear();
	triangles.ensureCapacity(MathUtil::max((int) 0, (int) vertexCount - 2) << 2);

	while (vertexCount > 3) {
		size_t previous = vertexCount - 1, i = 0, next = 1;

		while (true) {
			if (!isConcaveArray[i]) {
				int p1 = indices[previous] << 1, p2 = indices[i] << 1, p3 = indices[next] << 1;
				float p1x = vertices[p1], p1y = vertices[p1 + 1];
				float p2x = vertices[p2], p2y = vertices[p2 + 1];
				float p3x = vertices[p3], p3y = vertices[p3 + 1];
				for (size_t ii = (next + 1) % vertexCount; ii != previous; ii = (ii + 1) % vertexCount) {
					if (!isConcaveArray[ii]) continue;

					int v = indices[ii] << 1;
					float &vx = vertices[v], vy = vertices[v + 1];
					if (positiveArea(p3x, p3y, p1x, p1y, vx, vy)) {
						if (positiveArea(p1x, p1y, p2x, p2y, vx, vy)) {
							if (positiveArea(p2x, p2y, p3x, p3y, vx, vy)) {
								goto break_outer;
							}
						}
					}
				}
				break;
			}
		break_outer:

			if (next == 0) {
				do {
					if (!isConcaveArray[i]) break;
					i--;
				} while (i > 0);
				break;
			}

			previous = i;
			i = next;
			next = (next + 1) % vertexCount;
		}

		triangles.add(indices[(vertexCount + i - 1) % vertexCount]);
		triangles.add(indices[i]);
		triangles.add(indices[(i + 1) % vertexCount]);
		indices.removeAt(i);
		isConcaveArray.removeAt(i);
		vertexCount--;

		int previousIndex = (vertexCount + i - 1) % vertexCount;
		int nextIndex = i == vertexCount ? 0 : i;
		isConcaveArray[previousIndex] = isConcave(previousIndex, vertexCount, vertices, indices);
		isConcaveArray[nextIndex] = isConcave(nextIndex, vertexCount, vertices, indices);
	}

	if (vertexCount == 3) {
		triangles.add(indices[2]);
		triangles.add(indices[0]);
		triangles.add(indices[1]);
	}

	return triangles;
}

Vector<Vector<float> *> &Triangulator::decompose(Vector<float> &vertices, Vector<int> &triangles) {
	Vector<Vector<float> *> &convexPolygons = _convexPolygons;
	for (size_t i = 0, n = convexPolygons.size(); i < n; ++i)
		_polygonPool.free(convexPolygons[i]);
	convexPolygons.clear();

	Vector<Vector<int> *> &convexPolygonsIndices = _convexPolygonsIndices;
	for (size_t i = 0, n = convexPolygonsIndices.size(); i < n; ++i)
		_polygonIndicesPool.free(convexPolygonsIndices[i]);
	convexPolygonsIndices.clear();

	Vector<int> *polygonIndices = _polygonIndicesPool.obtain();
	polygonIndices->clear();

	Vector<float> *polygon = _polygonPool.obtain();
	polygon->clear();

	int fanBaseIndex = -1, lastwinding = 0;
	for (size_t i = 0, n = triangles.size(); i < n; i += 3) {
		int t1 = triangles[i] << 1, t2 = triangles[i + 1] << 1, t3 = triangles[i + 2] << 1;
		float x1 = vertices[t1], y1 = vertices[t1 + 1];
		float x2 = vertices[t2], y2 = vertices[t2 + 1];
		float x3 = vertices[t3], y3 = vertices[t3 + 1];

		bool merged = false;
		if (fanBaseIndex == t1) {
			size_t o = polygon->size() - 4;
			Vector<float> &p = *polygon;
			int winding1 = winding(p[o], p[o + 1], p[o + 2], p[o + 3], x3, y3);
			int winding2 = winding(x3, y3, p[0], p[1], p[2], p[3]);
			if (winding1 == lastwinding && winding2 == lastwinding) {
				polygon->add(x3);
				polygon->add(y3);
				polygonIndices->add(t3);
				merged = true;
			}
		}

		if (!merged) {
			if (polygon->size() > 0) {
				convexPolygons.add(polygon);
				convexPolygonsIndices.add(polygonIndices);
			} else {
				_polygonPool.free(polygon);
				_polygonIndicesPool.free(polygonIndices);
			}

			polygon = _polygonPool.obtain();
			polygon->clear();
			polygon->add(x1);
			polygon->add(y1);
			polygon->add(x2);
			polygon->add(y2);
			polygon->add(x3);
			polygon->add(y3);
			polygonIndices = _polygonIndicesPool.obtain();
			polygonIndices->clear();
			polygonIndices->add(t1);
			polygonIndices->add(t2);
			polygonIndices->add(t3);
			lastwinding = winding(x1, y1, x2, y2, x3, y3);
			fanBaseIndex = t1;
		}
	}

	if (polygon->size() > 0) {
		convexPolygons.add(polygon);
		convexPolygonsIndices.add(polygonIndices);
	}

	for (size_t i = 0, n = convexPolygons.size(); i < n; ++i) {
		polygonIndices = convexPolygonsIndices[i];

		if (polygonIndices->size() == 0) continue;
		int firstIndex = (*polygonIndices)[0];
		int lastIndex = (*polygonIndices)[polygonIndices->size() - 1];

		polygon = convexPolygons[i];
		size_t o = polygon->size() - 4;
		Vector<float> &p = *polygon;
		float prevPrevX = p[o], prevPrevY = p[o + 1];
		float prevX = p[o + 2], prevY = p[o + 3];
		float firstX = p[0], firstY = p[1];
		float secondX = p[2], secondY = p[3];
		int winding0 = winding(prevPrevX, prevPrevY, prevX, prevY, firstX, firstY);

		for (size_t ii = 0; ii < n; ++ii) {
			if (ii == i) continue;

			Vector<int> *otherIndicesP = convexPolygonsIndices[ii];
			Vector<int> &otherIndices = *otherIndicesP;

			if (otherIndices.size() != 3) continue;

			int otherFirstIndex = otherIndices[0];
			int otherSecondIndex = otherIndices[1];
			int otherLastIndex = otherIndices[2];

			Vector<float> *otherPolyP = convexPolygons[ii];
			Vector<float> &otherPoly = *otherPolyP;

			float x3 = otherPoly[otherPoly.size() - 2], y3 = otherPoly[otherPoly.size() - 1];

			if (otherFirstIndex != firstIndex || otherSecondIndex != lastIndex) continue;

			int winding1 = winding(prevPrevX, prevPrevY, prevX, prevY, x3, y3);
			int winding2 = winding(x3, y3, firstX, firstY, secondX, secondY);
			if (winding1 == winding0 && winding2 == winding0) {
				otherPoly.clear();
				otherIndices.clear();
				polygon->add(x3);
				polygon->add(y3);
				polygonIndices->add(otherLastIndex);
				prevPrevX = prevX;
				prevPrevY = prevY;
				prevX = x3;
				prevY = y3;
				ii = 0;
			}
		}
	}

	for (int i = (int) convexPolygons.size() - 1; i >= 0; --i) {
		polygon = convexPolygons[i];
		if (polygon->size() == 0) {
			convexPolygons.removeAt(i);
			_polygonPool.free(polygon);
			polygonIndices = convexPolygonsIndices[i];
			convexPolygonsIndices.removeAt(i);
			_polygonIndicesPool.free(polygonIndices);
		}
	}

	return convexPolygons;
}

bool Triangulator::isConcave(int index, int vertexCount, Vector<float> &vertices, Vector<int> &indices) {
	int previous = indices[(vertexCount + index - 1) % vertexCount] << 1;
	int current = indices[index] << 1;
	int next = indices[(index + 1) % vertexCount] << 1;

	return !positiveArea(vertices[previous], vertices[previous + 1],
						 vertices[current], vertices[current + 1],
						 vertices[next], vertices[next + 1]);
}

bool Triangulator::positiveArea(float p1x, float p1y, float p2x, float p2y, float p3x, float p3y) {
	return p1x * (p3y - p2y) + p2x * (p1y - p3y) + p3x * (p2y - p1y) >= 0;
}

int Triangulator::winding(float p1x, float p1y, float p2x, float p2y, float p3x, float p3y) {
	float px = p2x - p1x, py = p2y - p1y;
	return p3x * py - p3y * px + px * p1y - p1x * py >= 0 ? 1 : -1;
}
