#ifndef Spine_SkeletonClipping_h
#define Spine_SkeletonClipping_h

#include <spine/Array.h>
#include <spine/Triangulator.h>

namespace spine {
	class Slot;
	class Skeleton;
	class ClippingAttachment;

	class SP_API SkeletonClipping : public SpineObject {
	public:
		SkeletonClipping();

		size_t clipStart(Skeleton &skeleton, Slot &slot, ClippingAttachment *clip);

		void clipEnd(Slot &slot);

		void clipEnd();

		bool clipTriangles(float *vertices, unsigned short *triangles, size_t trianglesLength);

		bool clipTriangles(float *vertices, unsigned short *triangles, size_t trianglesLength, float *uvs, size_t stride);

		bool clipTriangles(Array<float> &vertices, Array<unsigned short> &triangles, Array<float> &uvs, size_t stride);

		bool isClipping();

		Array<float> &getClippedVertices();

		Array<unsigned short> &getClippedTriangles();

		Array<float> &getClippedUVs();

	private:
		Triangulator _triangulator;
		Array<float> _clippingPolygon;
		Array<Array<float> *> _clippingPolygons;
		Array<float> _clipOutput;
		Array<float> _clippedVertices;
		Array<unsigned short> _clippedTriangles;
		Array<float> _clippedUVs;
		Array<float> _inverseVertices;
		Array<float> _scratch;
		ClippingAttachment *_clipAttachment;
		bool _inverse;

		bool clip(float x1, float y1, float x2, float y2, float x3, float y3, Array<float> *polygon);

		void clipInverse(float x1, float y1, float x2, float y2, float x3, float y3, Array<float> *polygon);

		static bool makeClockwise(Array<float> &polygon);

		void makeConvex(Array<float> &polygon);
	};
}

#endif
