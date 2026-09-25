
#include <spine/ClippingAttachment.h>
#include <spine/extension.h>

void _spClippingAttachment_dispose (spAttachment* attachment) {
	spClippingAttachment* self = SUB_CAST(spClippingAttachment, attachment);

	_spVertexAttachment_deinit(SUPER(self));

	FREE(self);
}

spClippingAttachment* spClippingAttachment_create (const char* name) {
	spClippingAttachment* self = NEW(spClippingAttachment);
	_spVertexAttachment_init(SUPER(self));
	_spAttachment_init(SUPER(SUPER(self)), name, SP_ATTACHMENT_CLIPPING, _spClippingAttachment_dispose);
	self->endSlot = 0;
	return self;
}
