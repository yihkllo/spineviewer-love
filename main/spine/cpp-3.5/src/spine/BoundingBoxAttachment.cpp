
#include <spine/BoundingBoxAttachment.h>
#include <spine/extension.h>

void _spBoundingBoxAttachment_dispose (spAttachment* attachment) {
	spBoundingBoxAttachment* self = SUB_CAST(spBoundingBoxAttachment, attachment);

	_spVertexAttachment_deinit(SUPER(self));

	FREE(self);
}

spBoundingBoxAttachment* spBoundingBoxAttachment_create (const char* name) {
	spBoundingBoxAttachment* self = NEW(spBoundingBoxAttachment);
	_spVertexAttachment_init(SUPER(self));
	_spAttachment_init(SUPER(SUPER(self)), name, SP_ATTACHMENT_BOUNDING_BOX, _spBoundingBoxAttachment_dispose);
	return self;
}
