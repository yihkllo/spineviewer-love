
#ifndef SPINE_SKELETONBOUNDS_H_
#define SPINE_SKELETONBOUNDS_H_

#include <spine/BoundingBoxAttachment.h>
#include <spine/Skeleton.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spPolygon {
	float* const vertices;
	int count;
	int capacity;
} spPolygon;

spPolygon* spPolygon_create (int capacity);
void spPolygon_dispose (spPolygon* self);

int spPolygon_containsPoint (spPolygon* polygon, float x, float y);
int spPolygon_intersectsSegment (spPolygon* polygon, float x1, float y1, float x2, float y2);

#ifdef SPINE_SHORT_NAMES
typedef spPolygon Polygon;
#define Polygon_create(...) spPolygon_create(__VA_ARGS__)
#define Polygon_dispose(...) spPolygon_dispose(__VA_ARGS__)
#define Polygon_containsPoint(...) spPolygon_containsPoint(__VA_ARGS__)
#define Polygon_intersectsSegment(...) spPolygon_intersectsSegment(__VA_ARGS__)
#endif


typedef struct spSkeletonBounds {
	int count;
	spBoundingBoxAttachment** boundingBoxes;
	spPolygon** polygons;

	float minX, minY, maxX, maxY;
} spSkeletonBounds;

spSkeletonBounds* spSkeletonBounds_create ();
void spSkeletonBounds_dispose (spSkeletonBounds* self);
void spSkeletonBounds_update (spSkeletonBounds* self, spSkeleton* skeleton, int updateAabb);

int spSkeletonBounds_aabbContainsPoint (spSkeletonBounds* self, float x, float y);

int spSkeletonBounds_aabbIntersectsSegment (spSkeletonBounds* self, float x1, float y1, float x2, float y2);

int spSkeletonBounds_aabbIntersectsSkeleton (spSkeletonBounds* self, spSkeletonBounds* bounds);

spBoundingBoxAttachment* spSkeletonBounds_containsPoint (spSkeletonBounds* self, float x, float y);

spBoundingBoxAttachment* spSkeletonBounds_intersectsSegment (spSkeletonBounds* self, float x1, float y1, float x2, float y2);

spPolygon* spSkeletonBounds_getPolygon (spSkeletonBounds* self, spBoundingBoxAttachment* boundingBox);

#ifdef SPINE_SHORT_NAMES
typedef spSkeletonBounds SkeletonBounds;
#define SkeletonBounds_create(...) spSkeletonBounds_create(__VA_ARGS__)
#define SkeletonBounds_dispose(...) spSkeletonBounds_dispose(__VA_ARGS__)
#define SkeletonBounds_update(...) spSkeletonBounds_update(__VA_ARGS__)
#define SkeletonBounds_aabbContainsPoint(...) spSkeletonBounds_aabbContainsPoint(__VA_ARGS__)
#define SkeletonBounds_aabbIntersectsSegment(...) spSkeletonBounds_aabbIntersectsSegment(__VA_ARGS__)
#define SkeletonBounds_aabbIntersectsSkeleton(...) spSkeletonBounds_aabbIntersectsSkeleton(__VA_ARGS__)
#define SkeletonBounds_containsPoint(...) spSkeletonBounds_containsPoint(__VA_ARGS__)
#define SkeletonBounds_intersectsSegment(...) spSkeletonBounds_intersectsSegment(__VA_ARGS__)
#define SkeletonBounds_getPolygon(...) spSkeletonBounds_getPolygon(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
