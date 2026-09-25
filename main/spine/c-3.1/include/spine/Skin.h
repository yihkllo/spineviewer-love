
#ifndef SPINE_SKIN_H_
#define SPINE_SKIN_H_

#include <spine/Attachment.h>

#ifdef __cplusplus
extern "C" {
#endif

struct spSkeleton;

typedef struct spSkin {
	const char* const name;

#ifdef __cplusplus
	spSkin() :
		name(0) {
	}
#endif
} spSkin;

spSkin* spSkin_create (const char* name);
void spSkin_dispose (spSkin* self);

void spSkin_addAttachment (spSkin* self, int slotIndex, const char* name, spAttachment* attachment);
spAttachment* spSkin_getAttachment (const spSkin* self, int slotIndex, const char* name);

const char* spSkin_getAttachmentName (const spSkin* self, int slotIndex, int attachmentIndex);

void spSkin_attachAll (const spSkin* self, struct spSkeleton* skeleton, const spSkin* oldspSkin);

#ifdef SPINE_SHORT_NAMES
typedef spSkin Skin;
#define Skin_create(...) spSkin_create(__VA_ARGS__)
#define Skin_dispose(...) spSkin_dispose(__VA_ARGS__)
#define Skin_addAttachment(...) spSkin_addAttachment(__VA_ARGS__)
#define Skin_getAttachment(...) spSkin_getAttachment(__VA_ARGS__)
#define Skin_getAttachmentName(...) spSkin_getAttachmentName(__VA_ARGS__)
#define Skin_attachAll(...) spSkin_attachAll(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
