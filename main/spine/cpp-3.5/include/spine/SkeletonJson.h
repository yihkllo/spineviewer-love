
#ifndef SPINE_SKELETONJSON_H_
#define SPINE_SKELETONJSON_H_

#include <spine/Attachment.h>
#include <spine/AttachmentLoader.h>
#include <spine/SkeletonData.h>
#include <spine/Atlas.h>
#include <spine/Animation.h>

#ifdef __cplusplus
extern "C" {
#endif

struct spAtlasAttachmentLoader;

typedef struct spSkeletonJson {
	float scale;
	spAttachmentLoader* attachmentLoader;
	const char* const error;
} spSkeletonJson;

spSkeletonJson* spSkeletonJson_createWithLoader (spAttachmentLoader* attachmentLoader);
spSkeletonJson* spSkeletonJson_create (spAtlas* atlas);
void spSkeletonJson_dispose (spSkeletonJson* self);

spSkeletonData* spSkeletonJson_readSkeletonData (spSkeletonJson* self, const char* json);
spSkeletonData* spSkeletonJson_readSkeletonDataFile (spSkeletonJson* self, const char* path);

#ifdef SPINE_SHORT_NAMES
typedef spSkeletonJson SkeletonJson;
#define SkeletonJson_createWithLoader(...) spSkeletonJson_createWithLoader(__VA_ARGS__)
#define SkeletonJson_create(...) spSkeletonJson_create(__VA_ARGS__)
#define SkeletonJson_dispose(...) spSkeletonJson_dispose(__VA_ARGS__)
#define SkeletonJson_readSkeletonData(...) spSkeletonJson_readSkeletonData(__VA_ARGS__)
#define SkeletonJson_readSkeletonDataFile(...) spSkeletonJson_readSkeletonDataFile(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
