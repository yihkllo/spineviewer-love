
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
	_spAttachment_init(SUPER(SUPER(self)), name, SP_ATTACHMENT_PATH, _spPathAttachment_dispose);
	return self;
}

void spPathAttachment_computeWorldVertices (spPathAttachment* self, spSlot* slot, float* worldVertices) {
	spVertexAttachment_computeWorldVertices(SUPER(self), slot, worldVertices);
}

void spPathAttachment_computeWorldVertices1 (spPathAttachment* self, spSlot* slot, int start, int count, float* worldVertices, int offset) {
	spVertexAttachment_computeWorldVertices1(SUPER(self), start, count, slot, worldVertices, offset);
}
