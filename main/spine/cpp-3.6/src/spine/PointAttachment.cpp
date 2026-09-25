
#include <spine/PointAttachment.h>
#include <spine/extension.h>

void _spPointAttachment_dispose (spAttachment* attachment) {
	spPathAttachment* self = SUB_CAST(spPathAttachment, attachment);

	_spVertexAttachment_deinit(SUPER(self));

	FREE(self);
}

spPointAttachment* spPointAttachment_create (const char* name) {
	spPointAttachment* self = NEW(spPointAttachment);
	_spVertexAttachment_init(SUPER(self));
	_spAttachment_init(SUPER(SUPER(self)), name, SP_ATTACHMENT_POINT, _spPointAttachment_dispose);
	return self;
}

void spPointAttachment_computeWorldPosition (spPointAttachment* self, spBone* bone, float* x, float* y) {
	*x = self->x * bone->a + self->y * bone->b + bone->worldX;
	*y = self->x * bone->c + self->y * bone->d + bone->worldY;
}

float spPointAttachment_computeWorldRotation (spPointAttachment* self, spBone* bone) {
	float cosine, sine, x, y;
	cosine = COS_DEG(self->rotation);
	sine = SIN_DEG(self->rotation);
	x = cosine * bone->a + sine * bone->b;
	y = cosine * bone->c + sine * bone->d;
	return ATAN2(y, x) * RAD_DEG;
}
