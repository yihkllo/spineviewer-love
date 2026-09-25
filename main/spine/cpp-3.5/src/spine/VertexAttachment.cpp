
#include <spine/VertexAttachment.h>
#include <spine/extension.h>

static int nextID = 0;

void _spVertexAttachment_init (spVertexAttachment* attachment) {
	attachment->id = (nextID++ & 65535) << 11;
}

void _spVertexAttachment_deinit (spVertexAttachment* attachment) {
	_spAttachment_deinit(SUPER(attachment));
	FREE(attachment->bones);
	FREE(attachment->vertices);
}

void spVertexAttachment_computeWorldVertices (spVertexAttachment* self, spSlot* slot, int start, int count, float* worldVertices, int offset, int stride) {
	spSkeleton* skeleton;
	int deformLength;
	float* deform;
	float* vertices;
	int* bones;

	count += offset;
	skeleton = slot->bone->skeleton;
	deformLength = slot->attachmentVerticesCount;
	deform = slot->attachmentVertices;
	vertices = self->vertices;
	bones = self->bones;
	if (!bones) {
		spBone* bone;
		int v, w;
		float x, y;
		if (deformLength > 0) vertices = deform;
		bone = slot->bone;
		x = bone->worldX;
		y = bone->worldY;
		for (v = start, w = offset; w < count; v += 2, w += stride) {
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
			for (w = offset, b = skip * 3; w < count; w += stride) {
				float wx = 0, wy = 0;
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
			for (w = offset, b = skip * 3, f = skip << 1; w < count; w += stride) {
				float wx = 0, wy = 0;
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
