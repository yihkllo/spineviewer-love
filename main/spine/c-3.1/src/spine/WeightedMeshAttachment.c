
#include <spine/WeightedMeshAttachment.h>
#include <spine/extension.h>

void _spWeightedMeshAttachment_dispose (spAttachment* attachment) {
	spWeightedMeshAttachment* self = SUB_CAST(spWeightedMeshAttachment, attachment);
	_spAttachment_deinit(attachment);
	FREE(self->path);
	FREE(self->uvs);
	if (!self->parentMesh) {
		FREE(self->regionUVs);
		FREE(self->bones);
		FREE(self->weights);
		FREE(self->triangles);
		FREE(self->edges);
	}
	FREE(self);
}

spWeightedMeshAttachment* spWeightedMeshAttachment_create (const char* name) {
	spWeightedMeshAttachment* self = NEW(spWeightedMeshAttachment);
	self->r = 1;
	self->g = 1;
	self->b = 1;
	self->a = 1;
	_spAttachment_init(SUPER(self), name, SP_ATTACHMENT_WEIGHTED_MESH, _spWeightedMeshAttachment_dispose);
	return self;
}

void spWeightedMeshAttachment_updateUVs (spWeightedMeshAttachment* self) {
	int i;
	float width = self->regionU2 - self->regionU, height = self->regionV2 - self->regionV;
	FREE(self->uvs);
	self->uvs = MALLOC(float, self->uvsCount);
	if (self->regionRotate) {
		for (i = 0; i < self->uvsCount; i += 2) {
			self->uvs[i] = self->regionU + self->regionUVs[i + 1] * width;
			self->uvs[i + 1] = self->regionV + height - self->regionUVs[i] * height;
		}
	} else {
		for (i = 0; i < self->uvsCount; i += 2) {
			self->uvs[i] = self->regionU + self->regionUVs[i] * width;
			self->uvs[i + 1] = self->regionV + self->regionUVs[i + 1] * height;
		}
	}
}

void spWeightedMeshAttachment_computeWorldVertices (spWeightedMeshAttachment* self, spSlot* slot, float* worldVertices) {
	int w = 0, v = 0, b = 0, f = 0;
	float x = slot->bone->skeleton->x, y = slot->bone->skeleton->y;
	spBone** skeletonBones = slot->bone->skeleton->bones;
	if (slot->attachmentVerticesCount == 0) {
		for (; v < self->bonesCount; w += 2) {
			float wx = 0, wy = 0;
			const int nn = self->bones[v] + v;
			v++;
			for (; v <= nn; v++, b += 3) {
				const spBone* bone = skeletonBones[self->bones[v]];
				const float vx = self->weights[b], vy = self->weights[b + 1], weight = self->weights[b + 2];
				wx += (vx * bone->a + vy * bone->b + bone->worldX) * weight;
				wy += (vx * bone->c + vy * bone->d + bone->worldY) * weight;
			}
			worldVertices[w] = wx + x;
			worldVertices[w + 1] = wy + y;
		}
	} else {
		const float* ffd = slot->attachmentVertices;
		for (; v < self->bonesCount; w += 2) {
			float wx = 0, wy = 0;
			const int nn = self->bones[v] + v;
			v++;
			for (; v <= nn; v++, b += 3, f += 2) {
				const spBone* bone = skeletonBones[self->bones[v]];
				const float vx = self->weights[b] + ffd[f], vy = self->weights[b + 1] + ffd[f + 1], weight = self->weights[b + 2];
				wx += (vx * bone->a + vy * bone->b + bone->worldX) * weight;
				wy += (vx * bone->c + vy * bone->d + bone->worldY) * weight;
			}
			worldVertices[w] = wx + x;
			worldVertices[w + 1] = wy + y;
		}
	}
}

void spWeightedMeshAttachment_setParentMesh (spWeightedMeshAttachment* self, spWeightedMeshAttachment* parentMesh) {
	CONST_CAST(spWeightedMeshAttachment*, self->parentMesh) = parentMesh;
	if (parentMesh) {
		self->bones = parentMesh->bones;
		self->bonesCount = parentMesh->bonesCount;

		self->weights = parentMesh->weights;
		self->weightsCount = parentMesh->weightsCount;

		self->regionUVs = parentMesh->regionUVs;
		self->uvsCount = parentMesh->uvsCount;

		self->triangles = parentMesh->triangles;
		self->trianglesCount = parentMesh->trianglesCount;

		self->hullLength = parentMesh->hullLength;

		self->edges = parentMesh->edges;
		self->edgesCount = parentMesh->edgesCount;

		self->width = parentMesh->width;
		self->height = parentMesh->height;
	}
}
