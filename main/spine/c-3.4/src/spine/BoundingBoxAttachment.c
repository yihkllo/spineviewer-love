
#include <spine/BoundingBoxAttachment.h>
#include <spine/extension.h>

void _spBoundingBoxAttachment_dispose (spAttachment* attachment) {
	spBoundingBoxAttachment* self = SUB_CAST(spBoundingBoxAttachment, attachment);

	_spVertexAttachment_deinit(SUPER(self));

	FREE(self);
}

spBoundingBoxAttachment* spBoundingBoxAttachment_create (const char* name) {
	spBoundingBoxAttachment* self = NEW(spBoundingBoxAttachment);
	_spAttachment_init(SUPER(SUPER(self)), name, SP_ATTACHMENT_BOUNDING_BOX, _spBoundingBoxAttachment_dispose);
	return self;
}

void spBoundingBoxAttachment_computeWorldVertices (spBoundingBoxAttachment* self, spSlot* slot, float* worldVertices) {
	spVertexAttachment_computeWorldVertices(SUPER(self), slot, worldVertices);
}
