
#ifndef SPINE_ATTACHMENTLOADER_H_
#define SPINE_ATTACHMENTLOADER_H_

#include <spine/dll.h>
#include <spine/Attachment.h>
#include <spine/Skin.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct spAttachmentLoader {
	const char* error1;
	const char* error2;

	const void* const vtable;
#ifdef __cplusplus
	spAttachmentLoader () :
					error1(0),
					error2(0),
					vtable(0) {
	}
#endif
} spAttachmentLoader;

SP_API void spAttachmentLoader_dispose (spAttachmentLoader* self);

SP_API spAttachment* spAttachmentLoader_createAttachment (spAttachmentLoader* self, spSkin* skin, spAttachmentType type, const char* name,
		const char* path);
SP_API void spAttachmentLoader_configureAttachment (spAttachmentLoader* self, spAttachment* attachment);
SP_API void spAttachmentLoader_disposeAttachment (spAttachmentLoader* self, spAttachment* attachment);

#ifdef SPINE_SHORT_NAMES
typedef spAttachmentLoader AttachmentLoader;
#define AttachmentLoader_dispose(...) spAttachmentLoader_dispose(__VA_ARGS__)
#define AttachmentLoader_createAttachment(...) spAttachmentLoader_createAttachment(__VA_ARGS__)
#define AttachmentLoader_configureAttachment(...) spAttachmentLoader_configureAttachment(__VA_ARGS__)
#define AttachmentLoader_disposeAttachment(...) spAttachmentLoader_disposeAttachment(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
