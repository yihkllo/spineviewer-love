
#ifndef SPINE_ATTACHMENT_H_
#define SPINE_ATTACHMENT_H_

#ifdef __cplusplus
extern "C" {
#endif

struct spAttachmentLoader;

typedef enum {
	SP_ATTACHMENT_REGION,
	SP_ATTACHMENT_BOUNDING_BOX,
	SP_ATTACHMENT_MESH,
	SP_ATTACHMENT_WEIGHTED_MESH,
	SP_ATTACHMENT_LINKED_MESH,
	SP_ATTACHMENT_WEIGHTED_LINKED_MESH
} spAttachmentType;

typedef struct spAttachment {
	const char* const name;
	const spAttachmentType type;
	const void* const vtable;
	struct spAttachmentLoader* attachmentLoader;

#ifdef __cplusplus
	spAttachment() :
		name(0),
		type(SP_ATTACHMENT_REGION),
		vtable(0) {
	}
#endif
} spAttachment;

void spAttachment_dispose (spAttachment* self);

#ifdef SPINE_SHORT_NAMES
typedef spAttachmentType AttachmentType;
#define ATTACHMENT_REGION SP_ATTACHMENT_REGION
#define ATTACHMENT_BOUNDING_BOX SP_ATTACHMENT_BOUNDING_BOX
#define ATTACHMENT_MESH SP_ATTACHMENT_MESH
#define ATTACHMENT_WEIGHTED_MESH SP_ATTACHMENT_WEIGHTED_MESH
#define ATTACHMENT_LINKED_MESH SP_ATTACHMENT_LINKED_MESH
#define ATTACHMENT_WEIGHTED_LINKED_MESH SP_ATTACHMENT_WEIGHTED_LINKED_MESH
typedef spAttachment Attachment;
#define Attachment_dispose(...) spAttachment_dispose(__VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif
