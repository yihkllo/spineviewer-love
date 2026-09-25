
#include <spine/BoundingBoxAttachment.h>
#include <spine/extension.h>

void _spBoundingBoxAttachment_dispose (spAttachment* attachment) {
	spBoundingBoxAttachment* self = SUB_CAST(spBoundingBoxAttachment, attachment);

	_spAttachment_deinit(attachment);

	FREE(self->vertices);
	FREE(self);
}

spBoundingBoxAttachment* spBoundingBoxAttachment_create (const char* name) {
	spBoundingBoxAttachment* self = NEW(spBoundingBoxAttachment);
	_spAttachment_init(SUPER(self), name, SP_ATTACHMENT_BOUNDING_BOX, _spBoundingBoxAttachment_dispose);
	return self;
}

void spBoundingBoxAttachment_computeWorldVertices (spBoundingBoxAttachment* self, spBone* bone, float* worldVertices) {
	int i;
	float px, py;
	float* vertices = self->vertices;
	float x = bone->skeleton->x + bone->worldX, y = bone->skeleton->y + bone->worldY;
	for (i = 0; i < self->verticesCount; i += 2) {
		px = vertices[i];
		py = vertices[i + 1];
		worldVertices[i] = px * bone->a + py * bone->b + x;
		worldVertices[i + 1] = px * bone->c + py * bone->d + y;
	}
}
