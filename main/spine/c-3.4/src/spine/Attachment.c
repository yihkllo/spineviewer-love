
#include <spine/Attachment.h>
#include <spine/extension.h>
#include <spine/Slot.h>

typedef struct _spAttachmentVtable {
	void (*dispose) (spAttachment* self);
} _spAttachmentVtable;

void _spAttachment_init (spAttachment* self, const char* name, spAttachmentType type,
		void (*dispose) (spAttachment* self)) {

	CONST_CAST(_spAttachmentVtable*, self->vtable) = NEW(_spAttachmentVtable);
	VTABLE(spAttachment, self) ->dispose = dispose;

	MALLOC_STR(self->name, name);
	CONST_CAST(spAttachmentType, self->type) = type;
}

void _spAttachment_deinit (spAttachment* self) {
	if (self->attachmentLoader) spAttachmentLoader_disposeAttachment(self->attachmentLoader, self);
	FREE(self->vtable);
	FREE(self->name);
}

void spAttachment_dispose (spAttachment* self) {
	VTABLE(spAttachment, self) ->dispose(self);
}
