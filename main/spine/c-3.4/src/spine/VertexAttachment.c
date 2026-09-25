
#include <spine/VertexAttachment.h>
#include <spine/extension.h>

void _spVertexAttachment_deinit (spVertexAttachment* attachment) {
	_spAttachment_deinit(SUPER(attachment));
	FREE(attachment->bones);
	FREE(attachment->vertices);
}

void spVertexAttachment_computeWorldVertices (spVertexAttachment* self, spSlot* slot, float* worldVertices) {
	spVertexAttachment_computeWorldVertices1(self, 0, self->worldVerticesLength, slot, worldVertices, 0);
}

void spVertexAttachment_computeWorldVertices1 (spVertexAttachment* self, int start, int count, spSlot* slot, float* worldVertices, int offset) {
	spSkeleton* skeleton;
	float x, y;
	int deformLength;
	float* deform;
	float* vertices;
	int* bones;

	count += offset;
	skeleton = slot->bone->skeleton;
	x = skeleton->x;
	y = skeleton->y;
	deformLength = slot->attachmentVerticesCount;
	deform = slot->attachmentVertices;
	vertices = self->vertices;
	bones = self->bones;
	if (!bones) {
		spBone* bone;
		int v, w;
		if (deformLength > 0) vertices = deform;
		bone = slot->bone;
		x += bone->worldX;
		y += bone->worldY;
		for (v = start, w = offset; w < count; v += 2, w += 2) {
			float vx = vertices[v], vy = vertices[v + 1];
			worldVertices[w] = vx * bone->a + vy * bone->b + x;
			worldVertices[w + 1] = vx * bone->c + vy * bone->d + y;
		}
	} else {
		int v = 0, skip = 0, i;
		spBone** skeletonBones;
		for (i = 0; i < start; i += 2) {
			int n = bones[v];
			v += n + 1;
			skip += n;
		}
		skeletonBones = skeleton->bones;
		if (deformLength == 0) {
			int w, b;
			for (w = offset, b = skip * 3; w < count; w += 2) {
				float wx = x, wy = y;
				int n = bones[v++];
				n += v;
				for (; v < n; v++, b += 3) {
					spBone* bone = skeletonBones[bones[v]];
					float vx = vertices[b], vy = vertices[b + 1], weight = vertices[b + 2];
					wx += (vx * bone->a + vy * bone->b + bone->worldX) * weight;
					wy += (vx * bone->c + vy * bone->d + bone->worldY) * weight;
				}
				worldVertices[w] = wx;
				worldVertices[w + 1] = wy;
			}
		} else {
			int w, b, f;
			for (w = offset, b = skip * 3, f = skip << 1; w < count; w += 2) {
				float wx = x, wy = y;
				int n = bones[v++];
				n += v;
				for (; v < n; v++, b += 3, f += 2) {
					spBone* bone = skeletonBones[bones[v]];
					float vx = vertices[b] + deform[f], vy = vertices[b + 1] + deform[f + 1], weight = vertices[b + 2];
					wx += (vx * bone->a + vy * bone->b + bone->worldX) * weight;
					wy += (vx * bone->c + vy * bone->d + bone->worldY) * weight;
				}
				worldVertices[w] = wx;
				worldVertices[w + 1] = wy;
			}
		}
	}
}
