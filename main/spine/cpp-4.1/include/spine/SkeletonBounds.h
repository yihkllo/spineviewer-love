
#ifndef Spine_SkeletonBounds_h
#define Spine_SkeletonBounds_h

#include <spine/Vector.h>
#include <spine/Pool.h>
#include <spine/SpineObject.h>

namespace spine {
	class Skeleton;

	class BoundingBoxAttachment;

	class Polygon;

	class SP_API SkeletonBounds : public SpineObject {
	public:
		SkeletonBounds();

		~SkeletonBounds();

		void update(Skeleton &skeleton, bool updateAabb);

		bool aabbcontainsPoint(float x, float y);

		bool aabbintersectsSegment(float x1, float y1, float x2, float y2);

		bool aabbIntersectsSkeleton(SkeletonBounds bounds);

		bool containsPoint(Polygon *polygon, float x, float y);

		BoundingBoxAttachment *containsPoint(float x, float y);

		BoundingBoxAttachment *intersectsSegment(float x1, float y1, float x2, float y2);

		bool intersectsSegment(Polygon *polygon, float x1, float y1, float x2, float y2);

		Polygon *getPolygon(BoundingBoxAttachment *attachment);

        BoundingBoxAttachment * getBoundingBox(Polygon *polygon);

        Vector<Polygon *> &getPolygons();

        Vector<BoundingBoxAttachment *> &getBoundingBoxes();

		float getWidth();

		float getHeight();

	private:
		Pool <Polygon> _polygonPool;
		Vector<BoundingBoxAttachment *> _boundingBoxes;
		Vector<Polygon *> _polygons;
		float _minX, _minY, _maxX, _maxY;

		void aabbCompute();
	};

	class Polygon : public SpineObject {
	public:
		Vector<float> _vertices;
		int _count;

		Polygon() : _count(0) {
			_vertices.ensureCapacity(16);
		}
	};
}

#endif
