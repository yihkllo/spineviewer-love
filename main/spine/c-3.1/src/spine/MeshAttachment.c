
#include <spine/MeshAttachment.h>
#include <spine/extension.h>

void _spMeshAttachment_dispose (spAttachment* attachment) {
	spMeshAttachment* self = SUB_CAST(spMeshAttachment, attachment);
	_spAttachment_deinit(attachment);
	FREE(self->path);
	FREE(self->uvs);
	if (!self->parentMesh) {
		FREE(self->vertices);
		FREE(self->regionUVs);
		FREE(self->triangles);
		FREE(self->edges);
	}
	FREE(self);
}

spMeshAttachment* spMeshAttachment_create (const char* name) {
	spMeshAttachment* self = NEW(spMeshAttachment);
	self->r = 1;
	self->g = 1;
	self->b = 1;
	self->a = 1;
	_spAttachment_init(SUPER(self), name, SP_ATTACHMENT_MESH, _spMeshAttachment_dispose);
	return self;
}

void spMeshAttachment_updateUVs (spMeshAttachment* self) {
	int i;
	float width = self->regionU2 - self->regionU, height = self->regionV2 - self->regionV;
	FREE(self->uvs);
	self->uvs = MALLOC(float, self->verticesCount);
	if (self->regionRotate) {
		for (i = 0; i < self->verticesCount; i += 2) {
			self->uvs[i] = self->regionU + self->regionUVs[i + 1] * width;
			self->uvs[i + 1] = self->regionV + height - self->regionUVs[i] * height;
		}
	} else {
		for (i = 0; i < self->verticesCount; i += 2) {
			self->uvs[i] = self->regionU + self->regionUVs[i] * width;
			self->uvs[i + 1] = self->regionV + self->regionUVs[i + 1] * height;
		}
	}
}

void spMeshAttachment_computeWorldVertices (spMeshAttachment* self, spSlot* slot, float* worldVertices) {
	int i;
	float* vertices = self->vertices;
	const spBone* bone = slot->bone;
	float x = bone->skeleton->x + bone->worldX, y = bone->skeleton->y + bone->worldY;
	if (slot->attachmentVerticesCount == self->verticesCount) vertices = slot->attachmentVertices;
	for (i = 0; i < self->verticesCount; i += 2) {
		const float vx = vertices[i], vy = vertices[i + 1];
		worldVertices[i] = vx * bone->a + vy * bone->b + x;
		worldVertices[i + 1] = vx * bone->c + vy * bone->d + y;
	}
}

void spMeshAttachment_setParentMesh (spMeshAttachment* self, spMeshAttachment* parentMesh) {
	CONST_CAST(spMeshAttachment*, self->parentMesh) = parentMesh;
	if (parentMesh) {
		self->vertices = parentMesh->vertices;
		self->regionUVs = parentMesh->regionUVs;
		self->verticesCount = parentMesh->verticesCount;

		self->triangles = parentMesh->triangles;
		self->trianglesCount = parentMesh->trianglesCount;

		self->hullLength = parentMesh->hullLength;

		self->edges = parentMesh->edges;
		self->edgesCount = parentMesh->edgesCount;

		self->width = parentMesh->width;
		self->height = parentMesh->height;
	}
}
