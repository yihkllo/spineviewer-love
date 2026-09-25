
#ifndef Spine_SkeletonClipping_h
#define Spine_SkeletonClipping_h

#include <spine/Vector.h>
#include <spine/Triangulator.h>

namespace spine {
	class Slot;

	class ClippingAttachment;

	class SP_API SkeletonClipping : public SpineObject {
	public:
		SkeletonClipping();

		size_t clipStart(Slot &slot, ClippingAttachment *clip);

		void clipEnd(Slot &slot);

		void clipEnd();

		void
		clipTriangles(float *vertices, unsigned short *triangles, size_t trianglesLength, float *uvs, size_t stride);

		void
		clipTriangles(Vector<float> &vertices, Vector<unsigned short> &triangles, Vector<float> &uvs, size_t stride);

		bool isClipping();

		Vector<float> &getClippedVertices();

		Vector<unsigned short> &getClippedTriangles();

		Vector<float> &getClippedUVs();

	private:
		Triangulator _triangulator;
		Vector<float> _clippingPolygon;
		Vector<float> _clipOutput;
		Vector<float> _clippedVertices;
		Vector<unsigned short> _clippedTriangles;
		Vector<float> _clippedUVs;
		Vector<float> _scratch;
		ClippingAttachment *_clipAttachment;
		Vector<Vector<float> *> *_clippingPolygons;

		bool clip(float x1, float y1, float x2, float y2, float x3, float y3, Vector<float> *clippingArea,
				  Vector<float> *output);

		static void makeClockwise(Vector<float> &polygon);
	};
}

#endif
