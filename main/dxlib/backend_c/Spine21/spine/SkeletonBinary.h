#ifndef SPINE_SKELETONBINARY_H_
#define SPINE_SKELETONBINARY_H_

#include <spine/Attachment.h>
#include <spine/AttachmentLoader.h>
#include <spine/SkeletonData.h>
#include <spine/Atlas.h>
#include <spine/Animation.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spSkeletonBinary {
	float scale;
	spAttachmentLoader* attachmentLoader;
	const char* const error;
} spSkeletonBinary;

spSkeletonBinary* spSkeletonBinary_createWithLoader (spAttachmentLoader* attachmentLoader);
spSkeletonBinary* spSkeletonBinary_create (spAtlas* atlas);
void spSkeletonBinary_dispose (spSkeletonBinary* self);

spSkeletonData* spSkeletonBinary_readSkeletonData (spSkeletonBinary* self,
	const unsigned char* binary, int length);
spSkeletonData* spSkeletonBinary_readSkeletonDataFile (spSkeletonBinary* self,
	const char* path);

#ifdef SPINE_SHORT_NAMES
typedef spSkeletonBinary SkeletonBinary;
#define SkeletonBinary_createWithLoader(...) spSkeletonBinary_createWithLoader(__VA_ARGS__)
#define SkeletonBinary_create(...) spSkeletonBinary_create(__VA_ARGS__)
#define SkeletonBinary_dispose(...) spSkeletonBinary_dispose(__VA_ARGS__)
#define SkeletonBinary_readSkeletonData(...) spSkeletonBinary_readSkeletonData(__VA_ARGS__)
#define SkeletonBinary_readSkeletonDataFile(...) spSkeletonBinary_readSkeletonDataFile(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
