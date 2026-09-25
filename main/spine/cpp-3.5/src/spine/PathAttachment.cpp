
#include <spine/PathAttachment.h>
#include <spine/extension.h>

void _spPathAttachment_dispose (spAttachment* attachment) {
	spPathAttachment* self = SUB_CAST(spPathAttachment, attachment);

	_spVertexAttachment_deinit(SUPER(self));

	FREE(self->lengths);
	FREE(self);
}

spPathAttachment* spPathAttachment_create (const char* name) {
	spPathAttachment* self = NEW(spPathAttachment);
	_spVertexAttachment_init(SUPER(self));
	_spAttachment_init(SUPER(SUPER(self)), name, SP_ATTACHMENT_PATH, _spPathAttachment_dispose);
	return self;
}
