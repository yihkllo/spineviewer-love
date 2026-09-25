
#ifndef SPINE_ATLASATTACHMENTLOADER_H_
#define SPINE_ATLASATTACHMENTLOADER_H_

#include <spine/AttachmentLoader.h>
#include <spine/Atlas.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spAtlasAttachmentLoader {
	spAttachmentLoader super;
	spAtlas* atlas;
} spAtlasAttachmentLoader;

spAtlasAttachmentLoader* spAtlasAttachmentLoader_create (spAtlas* atlas);

#ifdef SPINE_SHORT_NAMES
typedef spAtlasAttachmentLoader AtlasAttachmentLoader;
#define AtlasAttachmentLoader_create(...) spAtlasAttachmentLoader_create(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
