

#include <spine/SkeletonBinary.h>
#include <spine/AtlasAttachmentLoader.h>
#include <spine/extension.h>

#include <string.h>
#include <stdint.h>


enum {
	BIN_TIMELINE_SCALE      = 0,
	BIN_TIMELINE_ROTATE     = 1,
	BIN_TIMELINE_TRANSLATE  = 2,
	BIN_TIMELINE_ATTACHMENT = 3,
	BIN_TIMELINE_COLOR      = 4,
	BIN_TIMELINE_FLIPX      = 5,
	BIN_TIMELINE_FLIPY      = 6
};

enum {
	BIN_CURVE_LINEAR  = 0,
	BIN_CURVE_STEPPED = 1,
	BIN_CURVE_BEZIER  = 2
};


typedef struct {
	const unsigned char* pos;
	const unsigned char* limit;
} BinaryStream;

typedef struct {
	spSkeletonBinary super;
	int ownsLoader;
} _spSkeletonBinaryInternal;


static unsigned char bin_read_u8(BinaryStream* s) {
	return *s->pos++;
}

static signed char bin_read_s8(BinaryStream* s) {
	return (signed char)bin_read_u8(s);
}

static int bin_read_bool(BinaryStream* s) {
	return bin_read_u8(s) != 0;
}

static int bin_read_int(BinaryStream* s) {

	uint32_t a = bin_read_u8(s);
	uint32_t b = bin_read_u8(s);
	uint32_t c = bin_read_u8(s);
	uint32_t d = bin_read_u8(s);
	return (int)((a << 24) | (b << 16) | (c << 8) | d);
}

static int bin_read_varint(BinaryStream* s, int optimizePositive) {
	unsigned char b = bin_read_u8(s);
	int32_t val = b & 0x7F;
	if (b & 0x80) {
		b = bin_read_u8(s);
		val |= (b & 0x7F) << 7;
		if (b & 0x80) {
			b = bin_read_u8(s);
			val |= (b & 0x7F) << 14;
			if (b & 0x80) {
				b = bin_read_u8(s);
				val |= (b & 0x7F) << 21;
				if (b & 0x80) {
					val |= (bin_read_u8(s) & 0x7F) << 28;
				}
			}
		}
	}
	if (!optimizePositive)
		val = (int32_t)(((uint32_t)val >> 1) ^ -(val & 1));
	return val;
}

static float bin_read_float(BinaryStream* s) {
	union { int i; float f; } conv;
	conv.i = bin_read_int(s);
	return conv.f;
}

static char* bin_read_string(BinaryStream* s) {
	int charCount = bin_read_varint(s, 1);
	char* out;
	int ci, byteLen;
	const unsigned char* start;
	if (charCount == 0) return 0;
	if (charCount == 1) {
		out = MALLOC(char, 1);
		out[0] = '\0';
		return out;
	}
	charCount--;


	start = s->pos;
	for (ci = 0; ci < charCount; ++ci) {
		unsigned char b = *s->pos++;
		if (b > 127) {

			if ((b >> 5) == 0x6)
				s->pos += 1;
			else if ((b >> 4) == 0xE)
				s->pos += 2;
			else if ((b >> 3) == 0x1E)
				s->pos += 3;
		}
	}
	byteLen = (int)(s->pos - start);
	out = MALLOC(char, byteLen + 1);
	memcpy(out, start, (size_t)byteLen);
	out[byteLen] = '\0';
	return out;
}


static float* bin_read_float_array(BinaryStream* s, int* outCount, float scale) {
	int n = bin_read_varint(s, 1);
	float* arr = MALLOC(float, n);
	int i;
	*outCount = n;
	if (scale == 1) {
		for (i = 0; i < n; ++i)
			arr[i] = bin_read_float(s);
	} else {
		for (i = 0; i < n; ++i)
			arr[i] = bin_read_float(s) * scale;
	}
	return arr;
}


static int* bin_read_short_array(BinaryStream* s, int* outCount) {
	int n = bin_read_varint(s, 1);
	int* arr = MALLOC(int, n);
	int i;
	*outCount = n;
	for (i = 0; i < n; ++i)
		arr[i] = (bin_read_u8(s) << 8) | bin_read_u8(s);
	return arr;
}

static int* bin_read_int_array(BinaryStream* s, int* outCount) {
	int n = bin_read_varint(s, 1);
	int* arr = MALLOC(int, n);
	int i;
	*outCount = n;
	for (i = 0; i < n; ++i)
		arr[i] = bin_read_varint(s, 1);
	return arr;
}


static void bin_decode_color(int packed, float* r, float* g, float* b, float* a) {
	*r = ((packed & 0xff000000u) >> 24) / 255.f;
	*g = ((packed & 0x00ff0000u) >> 16) / 255.f;
	*b = ((packed & 0x0000ff00u) >> 8)  / 255.f;
	*a = ((packed & 0x000000ffu))       / 255.f;
}


static void bin_set_error(spSkeletonBinary* self, const char* msg1, const char* msg2) {
	char buf[256];
	int len;
	FREE(self->error);
	strcpy(buf, msg1);
	len = (int)strlen(msg1);
	if (msg2) strncat(buf + len, msg2, 255 - len);
	MALLOC_STR(self->error, buf);
}


static void bin_read_curve(BinaryStream* s, spCurveTimeline* tl, int frame) {
	switch (bin_read_u8(s)) {
	case BIN_CURVE_STEPPED:
		spCurveTimeline_setStepped(tl, frame);
		break;
	case BIN_CURVE_BEZIER: {
		float cx1 = bin_read_float(s);
		float cy1 = bin_read_float(s);
		float cx2 = bin_read_float(s);
		float cy2 = bin_read_float(s);
		spCurveTimeline_setCurve(tl, frame, cx1, cy1, cx2, cy2);
		break;
	}
	}
}


static spAttachment* bin_read_attachment(spSkeletonBinary* self, BinaryStream* s,
		spSkin* skin, int slotIndex, const char* defaultName, int nonessential)
{
	float scale = self->scale;
	char* name = bin_read_string(s);
	int ownName = (name != 0);
	if (!name) {
		MALLOC_STR(name, defaultName);
		ownName = 1;
	}

	switch ((spAttachmentType)bin_read_u8(s)) {

	case SP_ATTACHMENT_REGION: {
		char* path = bin_read_string(s);
		spAttachment* att;
		spRegionAttachment* reg;
		int color;
		if (!path) MALLOC_STR(path, name);

		att = spAttachmentLoader_newAttachment(self->attachmentLoader, skin,
			SP_ATTACHMENT_REGION, name, path);
		if (ownName) FREE(name);
		if (!att) { FREE(path); return 0; }

		reg = SUB_CAST(spRegionAttachment, att);
		MALLOC_STR(reg->path, path);
		FREE(path);

		reg->x        = bin_read_float(s) * scale;
		reg->y        = bin_read_float(s) * scale;
		reg->scaleX   = bin_read_float(s);
		reg->scaleY   = bin_read_float(s);
		reg->rotation = bin_read_float(s);
		reg->width    = bin_read_float(s) * scale;
		reg->height   = bin_read_float(s) * scale;

		color = bin_read_int(s);
		bin_decode_color(color, &reg->r, &reg->g, &reg->b, &reg->a);

		spRegionAttachment_updateOffset(reg);
		return att;
	}

	case SP_ATTACHMENT_BOUNDING_BOX: {
		spAttachment* att;
		spBoundingBoxAttachment* box;
		int count, i;
		float* verts;

		att = spAttachmentLoader_newAttachment(self->attachmentLoader, skin,
			SP_ATTACHMENT_BOUNDING_BOX, name, 0);
		if (ownName) FREE(name);
		if (!att) return 0;

		box = SUB_CAST(spBoundingBoxAttachment, att);
		verts = bin_read_float_array(s, &count, scale);
		box->verticesCount = count;
		box->vertices = verts;
		return att;
	}

	case SP_ATTACHMENT_MESH: {
		char* path = bin_read_string(s);
		spAttachment* att;
		spMeshAttachment* mesh;
		int uvCount, triCount, vtxCount, color, i;
		if (!path) MALLOC_STR(path, name);

		att = spAttachmentLoader_newAttachment(self->attachmentLoader, skin,
			SP_ATTACHMENT_MESH, name, path);
		if (ownName) FREE(name);
		if (!att) { FREE(path); return 0; }

		mesh = SUB_CAST(spMeshAttachment, att);
		MALLOC_STR(mesh->path, path);
		FREE(path);


		mesh->regionUVs = bin_read_float_array(s, &uvCount, 1);


		mesh->triangles = bin_read_short_array(s, &triCount);
		mesh->trianglesCount = triCount;


		mesh->vertices = bin_read_float_array(s, &vtxCount, scale);
		mesh->verticesCount = vtxCount;

		spMeshAttachment_updateUVs(mesh);


		color = bin_read_int(s);
		bin_decode_color(color, &mesh->r, &mesh->g, &mesh->b, &mesh->a);

		mesh->hullLength = bin_read_varint(s, 1) * 2;

		if (nonessential) {
			mesh->edges = bin_read_int_array(s, &mesh->edgesCount);
			mesh->width  = bin_read_float(s) * scale;
			mesh->height = bin_read_float(s) * scale;
		}

		return att;
	}

	case SP_ATTACHMENT_SKINNED_MESH: {
		char* path = bin_read_string(s);
		spAttachment* att;
		spSkinnedMeshAttachment* mesh;
		int uvCount, triCount, rawCount, color, i, ri, bi, wi;
		float* rawVerts;
		if (!path) MALLOC_STR(path, name);

		att = spAttachmentLoader_newAttachment(self->attachmentLoader, skin,
			SP_ATTACHMENT_SKINNED_MESH, name, path);
		if (ownName) FREE(name);
		if (!att) { FREE(path); return 0; }

		mesh = SUB_CAST(spSkinnedMeshAttachment, att);
		MALLOC_STR(mesh->path, path);
		FREE(path);


		mesh->regionUVs = bin_read_float_array(s, &uvCount, 1);
		mesh->uvsCount = uvCount;


		mesh->triangles = bin_read_short_array(s, &triCount);
		mesh->trianglesCount = triCount;


		rawCount = bin_read_varint(s, 1);
		rawVerts = MALLOC(float, rawCount);
		for (i = 0; i < rawCount; ++i)
			rawVerts[i] = bin_read_float(s);


		mesh->bonesCount = 0;
		mesh->weightsCount = 0;
		for (ri = 0; ri < rawCount; ) {
			int bc = (int)rawVerts[ri];
			mesh->bonesCount += 1 + bc;
			mesh->weightsCount += bc * 3;
			ri += 1 + bc * 4;
		}

		mesh->bones   = MALLOC(int,   mesh->bonesCount);
		mesh->weights = MALLOC(float, mesh->weightsCount);


		for (ri = 0, bi = 0, wi = 0; ri < rawCount; ) {
			int bc = (int)rawVerts[ri++];
			int nn;
			mesh->bones[bi++] = bc;
			for (nn = ri + bc * 4; ri < nn; ri += 4) {
				mesh->bones[bi++]     = (int)rawVerts[ri];
				mesh->weights[wi]     = rawVerts[ri + 1] * scale;
				mesh->weights[wi + 1] = rawVerts[ri + 2] * scale;
				mesh->weights[wi + 2] = rawVerts[ri + 3];
				wi += 3;
			}
		}
		FREE(rawVerts);

		spSkinnedMeshAttachment_updateUVs(mesh);


		color = bin_read_int(s);
		bin_decode_color(color, &mesh->r, &mesh->g, &mesh->b, &mesh->a);

		mesh->hullLength = bin_read_varint(s, 1) * 2;

		if (nonessential) {
			mesh->edges = bin_read_int_array(s, &mesh->edgesCount);
			mesh->width  = bin_read_float(s) * scale;
			mesh->height = bin_read_float(s) * scale;
		}

		return att;
	}

	default:
		if (ownName) FREE(name);
		return 0;
	}
}


static spSkin* bin_read_skin(spSkeletonBinary* self, BinaryStream* s,
		const char* skinName, int nonessential)
{
	int slotCount = bin_read_varint(s, 1);
	int si;
	spSkin* skin;
	if (slotCount == 0) return 0;

	skin = spSkin_create(skinName);
	for (si = 0; si < slotCount; ++si) {
		int slotIndex = bin_read_varint(s, 1);
		int ai, attCount = bin_read_varint(s, 1);
		for (ai = 0; ai < attCount; ++ai) {
			const char* entryName = bin_read_string(s);
			spAttachment* att = bin_read_attachment(self, s, skin, slotIndex, entryName, nonessential);
			if (att) spSkin_addAttachment(skin, slotIndex, entryName, att);
			FREE(entryName);
		}
	}
	return skin;
}


#define INITIAL_TIMELINE_CAP 32

typedef struct {
	spTimeline** items;
	int count;
	int capacity;
} TimelineList;

static void tl_init(TimelineList* tl) {
	tl->capacity = INITIAL_TIMELINE_CAP;
	tl->count = 0;
	tl->items = MALLOC(spTimeline*, tl->capacity);
}

static void tl_push(TimelineList* tl, spTimeline* item) {
	if (tl->count == tl->capacity) {
		spTimeline** old = tl->items;
		tl->capacity *= 2;
		tl->items = MALLOC(spTimeline*, tl->capacity);
		memcpy(tl->items, old, sizeof(spTimeline*) * tl->count);
		FREE(old);
	}
	tl->items[tl->count++] = item;
}

static void tl_dispose(TimelineList* tl) {
	int i;
	for (i = 0; i < tl->count; ++i)
		spTimeline_dispose(tl->items[i]);
	FREE(tl->items);
}

static spAnimation* bin_read_animation(spSkeletonBinary* self, const char* name,
		BinaryStream* s, spSkeletonData* skeletonData)
{
	float scale = self->scale;
	float duration = 0;
	int i, n, ii, nn;
	spAnimation* anim;
	TimelineList tl;

	tl_init(&tl);


	for (i = 0, n = bin_read_varint(s, 1); i < n; ++i) {
		int slotIndex = bin_read_varint(s, 1);
		for (ii = 0, nn = bin_read_varint(s, 1); ii < nn; ++ii) {
			int type = bin_read_u8(s);
			int frameCount = bin_read_varint(s, 1);

			switch (type) {
			case BIN_TIMELINE_COLOR: {
				spColorTimeline* timeline = spColorTimeline_create(frameCount);
				int fi;
				timeline->slotIndex = slotIndex;
				for (fi = 0; fi < frameCount; ++fi) {
					float time = bin_read_float(s);
					int packed = bin_read_int(s);
					float r, g, b, a;
					bin_decode_color(packed, &r, &g, &b, &a);
					spColorTimeline_setFrame(timeline, fi, time, r, g, b, a);
					if (fi < frameCount - 1) bin_read_curve(s, SUPER(timeline), fi);
				}
				tl_push(&tl, SUPER_CAST(spTimeline, timeline));
				if (timeline->frames[frameCount * 5 - 5] > duration)
					duration = timeline->frames[frameCount * 5 - 5];
				break;
			}
			case BIN_TIMELINE_ATTACHMENT: {
				spAttachmentTimeline* timeline = spAttachmentTimeline_create(frameCount);
				int fi;
				timeline->slotIndex = slotIndex;
				for (fi = 0; fi < frameCount; ++fi) {
					float time = bin_read_float(s);
					const char* attName = bin_read_string(s);
					spAttachmentTimeline_setFrame(timeline, fi, time, attName);
					FREE(attName);
				}
				tl_push(&tl, SUPER_CAST(spTimeline, timeline));
				if (timeline->frames[frameCount - 1] > duration)
					duration = timeline->frames[frameCount - 1];
				break;
			}
			default:
				tl_dispose(&tl);
				bin_set_error(self, "Unknown slot timeline type: ", "");
				return 0;
			}
		}
	}


	for (i = 0, n = bin_read_varint(s, 1); i < n; ++i) {
		int boneIndex = bin_read_varint(s, 1);
		for (ii = 0, nn = bin_read_varint(s, 1); ii < nn; ++ii) {
			int type = bin_read_u8(s);
			int frameCount = bin_read_varint(s, 1);

			switch (type) {
			case BIN_TIMELINE_ROTATE: {
				spRotateTimeline* timeline = spRotateTimeline_create(frameCount);
				int fi;
				timeline->boneIndex = boneIndex;
				for (fi = 0; fi < frameCount; ++fi) {
					float time = bin_read_float(s);
					float angle = bin_read_float(s);
					spRotateTimeline_setFrame(timeline, fi, time, angle);
					if (fi < frameCount - 1) bin_read_curve(s, SUPER(timeline), fi);
				}
				tl_push(&tl, SUPER_CAST(spTimeline, timeline));
				if (timeline->frames[frameCount * 2 - 2] > duration)
					duration = timeline->frames[frameCount * 2 - 2];
				break;
			}
			case BIN_TIMELINE_TRANSLATE:
			case BIN_TIMELINE_SCALE: {
				spTranslateTimeline* timeline;
				float tlScale;
				int fi;

				if (type == BIN_TIMELINE_SCALE)
					timeline = spScaleTimeline_create(frameCount);
				else
					timeline = spTranslateTimeline_create(frameCount);

				tlScale = (type == BIN_TIMELINE_TRANSLATE) ? scale : 1;
				timeline->boneIndex = boneIndex;

				for (fi = 0; fi < frameCount; ++fi) {
					float time = bin_read_float(s);
					float x = bin_read_float(s) * tlScale;
					float y = bin_read_float(s) * tlScale;
					spTranslateTimeline_setFrame(timeline, fi, time, x, y);
					if (fi < frameCount - 1) bin_read_curve(s, SUPER(timeline), fi);
				}
				tl_push(&tl, SUPER_CAST(spTimeline, timeline));
				if (timeline->frames[frameCount * 3 - 3] > duration)
					duration = timeline->frames[frameCount * 3 - 3];
				break;
			}
			case BIN_TIMELINE_FLIPX:
			case BIN_TIMELINE_FLIPY: {
				spFlipTimeline* timeline = spFlipTimeline_create(frameCount,
					type == BIN_TIMELINE_FLIPX ? 1 : 0);
				int fi;
				timeline->boneIndex = boneIndex;
				for (fi = 0; fi < frameCount; ++fi) {
					float time = bin_read_float(s);
					int flip = bin_read_bool(s);
					spFlipTimeline_setFrame(timeline, fi, time, flip);
				}
				tl_push(&tl, SUPER_CAST(spTimeline, timeline));
				if (timeline->frames[frameCount * 2 - 2] > duration)
					duration = timeline->frames[frameCount * 2 - 2];
				break;
			}
			default:
				tl_dispose(&tl);
				bin_set_error(self, "Unknown bone timeline type: ", "");
				return 0;
			}
		}
	}


	for (i = 0, n = bin_read_varint(s, 1); i < n; ++i) {
		int ikIndex = bin_read_varint(s, 1);
		int frameCount = bin_read_varint(s, 1);
		spIkConstraintTimeline* timeline = spIkConstraintTimeline_create(frameCount);
		int fi;

		timeline->ikConstraintIndex = ikIndex;

		for (fi = 0; fi < frameCount; ++fi) {
			float time = bin_read_float(s);
			float mix = bin_read_float(s);
			signed char bend = bin_read_s8(s);
			spIkConstraintTimeline_setFrame(timeline, fi, time, mix, bend);
			if (fi < frameCount - 1) bin_read_curve(s, SUPER(timeline), fi);
		}
		tl_push(&tl, SUPER_CAST(spTimeline, timeline));
		if (timeline->frames[frameCount * 3 - 3] > duration)
			duration = timeline->frames[frameCount * 3 - 3];
	}


	for (i = 0, n = bin_read_varint(s, 1); i < n; ++i) {
		spSkin* skin = skeletonData->skins[bin_read_varint(s, 1)];
		for (ii = 0, nn = bin_read_varint(s, 1); ii < nn; ++ii) {
			int slotIndex = bin_read_varint(s, 1);
			int iii, nnn;
			for (iii = 0, nnn = bin_read_varint(s, 1); iii < nnn; ++iii) {
				const char* attachName = bin_read_string(s);
				spAttachment* attachment = spSkin_getAttachment(skin, slotIndex, attachName);
				int frameCount, vertexCount, fi;
				spFFDTimeline* timeline;
				float* scratch;

				if (!attachment) {
					tl_dispose(&tl);
					bin_set_error(self, "FFD attachment not found: ", attachName);
					FREE(attachName);
					return 0;
				}
				FREE(attachName);

				frameCount = bin_read_varint(s, 1);

				if (attachment->type == SP_ATTACHMENT_MESH)
					vertexCount = SUB_CAST(spMeshAttachment, attachment)->verticesCount;
				else
					vertexCount = SUB_CAST(spSkinnedMeshAttachment, attachment)->weightsCount / 3 * 2;

				timeline = spFFDTimeline_create(frameCount, vertexCount);
				timeline->slotIndex = slotIndex;
				timeline->attachment = attachment;

				scratch = MALLOC(float, vertexCount);

				for (fi = 0; fi < frameCount; ++fi) {
					float time = bin_read_float(s);
					float* frameVerts;
					int end = bin_read_varint(s, 1);

					if (end == 0) {

						if (attachment->type == SP_ATTACHMENT_MESH) {
							frameVerts = SUB_CAST(spMeshAttachment, attachment)->vertices;
						} else {
							memset(scratch, 0, sizeof(float) * vertexCount);
							frameVerts = scratch;
						}
					} else {
						int start = bin_read_varint(s, 1);
						int v;
						memset(scratch, 0, sizeof(float) * vertexCount);
						frameVerts = scratch;
						end += start;
						if (scale == 1) {
							for (v = start; v < end; ++v)
								scratch[v] = bin_read_float(s);
						} else {
							for (v = start; v < end; ++v)
								scratch[v] = bin_read_float(s) * scale;
						}

						if (attachment->type == SP_ATTACHMENT_MESH) {
							float* restVerts = SUB_CAST(spMeshAttachment, attachment)->vertices;
							for (v = 0; v < vertexCount; ++v)
								scratch[v] += restVerts[v];
						}
					}
					spFFDTimeline_setFrame(timeline, fi, time, frameVerts);
					if (fi < frameCount - 1) bin_read_curve(s, SUPER(timeline), fi);
				}
				FREE(scratch);

				tl_push(&tl, SUPER_CAST(spTimeline, timeline));
				if (timeline->frames[frameCount - 1] > duration)
					duration = timeline->frames[frameCount - 1];
			}
		}
	}


	{
		int drawOrderCount = bin_read_varint(s, 1);
		if (drawOrderCount > 0) {
			spDrawOrderTimeline* timeline = spDrawOrderTimeline_create(drawOrderCount, skeletonData->slotsCount);
			int di;
			for (di = 0; di < drawOrderCount; ++di) {
				int offsetCount = bin_read_varint(s, 1);
				int slotCount = skeletonData->slotsCount;
				int* drawOrder = MALLOC(int, slotCount);
				int* unchanged = MALLOC(int, slotCount - offsetCount);
				int origIdx = 0, unchIdx = 0;
				int oi, si;
				float time;

				for (si = slotCount - 1; si >= 0; --si)
					drawOrder[si] = -1;

				for (oi = 0; oi < offsetCount; ++oi) {
					int slotIdx = bin_read_varint(s, 1);
					while (origIdx != slotIdx)
						unchanged[unchIdx++] = origIdx++;
					drawOrder[origIdx + bin_read_varint(s, 1)] = origIdx;
					origIdx++;
				}
				while (origIdx < slotCount)
					unchanged[unchIdx++] = origIdx++;
				for (si = slotCount - 1; si >= 0; --si)
					if (drawOrder[si] == -1) drawOrder[si] = unchanged[--unchIdx];

				FREE(unchanged);
				time = bin_read_float(s);
				spDrawOrderTimeline_setFrame(timeline, di, time, drawOrder);
				FREE(drawOrder);
			}
			tl_push(&tl, SUPER_CAST(spTimeline, timeline));
			if (timeline->frames[drawOrderCount - 1] > duration)
				duration = timeline->frames[drawOrderCount - 1];
		}
	}


	{
		int eventCount = bin_read_varint(s, 1);
		if (eventCount > 0) {
			spEventTimeline* timeline = spEventTimeline_create(eventCount);
			int ei;
			for (ei = 0; ei < eventCount; ++ei) {
				float time = bin_read_float(s);
				spEventData* evData = skeletonData->events[bin_read_varint(s, 1)];
				spEvent* ev = spEvent_create(evData);
				ev->intValue = bin_read_varint(s, 0);
				ev->floatValue = bin_read_float(s);
				if (bin_read_bool(s)) {
					char* str = bin_read_string(s);
					MALLOC_STR(ev->stringValue, str);
					FREE(str);
				} else {
					MALLOC_STR(ev->stringValue, evData->stringValue);
				}
				spEventTimeline_setFrame(timeline, ei, time, ev);
			}
			tl_push(&tl, SUPER_CAST(spTimeline, timeline));
			if (timeline->frames[eventCount - 1] > duration)
				duration = timeline->frames[eventCount - 1];
		}
	}


	anim = spAnimation_create(name, tl.count);
	memcpy(anim->timelines, tl.items, sizeof(spTimeline*) * tl.count);
	anim->duration = duration;
	FREE(tl.items);

	return anim;
}


spSkeletonBinary* spSkeletonBinary_createWithLoader(spAttachmentLoader* loader) {
	spSkeletonBinary* self = SUPER(NEW(_spSkeletonBinaryInternal));
	self->scale = 1;
	self->attachmentLoader = loader;
	return self;
}

spSkeletonBinary* spSkeletonBinary_create(spAtlas* atlas) {
	spAtlasAttachmentLoader* loader = spAtlasAttachmentLoader_create(atlas);
	spSkeletonBinary* self = spSkeletonBinary_createWithLoader(SUPER(loader));
	SUB_CAST(_spSkeletonBinaryInternal, self)->ownsLoader = 1;
	return self;
}

void spSkeletonBinary_dispose(spSkeletonBinary* self) {
	if (SUB_CAST(_spSkeletonBinaryInternal, self)->ownsLoader)
		spAttachmentLoader_dispose(self->attachmentLoader);
	FREE(self->error);
	FREE(self);
}

spSkeletonData* spSkeletonBinary_readSkeletonDataFile(spSkeletonBinary* self, const char* path) {
	int length;
	spSkeletonData* result;
	const char* fileData = _spUtil_readFile(path, &length);
	if (!fileData) {
		bin_set_error(self, "Unable to read skeleton file: ", path);
		return 0;
	}
	result = spSkeletonBinary_readSkeletonData(self, (const unsigned char*)fileData, length);
	FREE(fileData);
	return result;
}

spSkeletonData* spSkeletonBinary_readSkeletonData(spSkeletonBinary* self,
		const unsigned char* data, int length)
{
	float scale = self->scale;
	BinaryStream stream;
	BinaryStream* s = &stream;
	spSkeletonData* sd;
	int i, nonessential;

	stream.pos   = data;
	stream.limit = data + length;

	FREE(self->error);
	CONST_CAST(char*, self->error) = 0;

	sd = spSkeletonData_create();


	sd->hash = bin_read_string(s);
	if (sd->hash && strlen(sd->hash) == 0) { FREE(sd->hash); sd->hash = 0; }

	sd->version = bin_read_string(s);
	if (sd->version && strlen(sd->version) == 0) { FREE(sd->version); sd->version = 0; }

	sd->width  = bin_read_float(s);
	sd->height = bin_read_float(s);

	nonessential = bin_read_bool(s);
	if (nonessential) {
		char* imgPath = bin_read_string(s);
		FREE(imgPath);
	}


	sd->bonesCount = bin_read_varint(s, 1);
	sd->bones = MALLOC(spBoneData*, sd->bonesCount);
	for (i = 0; i < sd->bonesCount; ++i) {
		const char* boneName = bin_read_string(s);
		int parentIdx = bin_read_varint(s, 1) - 1;
		spBoneData* parent = (parentIdx < 0) ? 0 : sd->bones[parentIdx];
		spBoneData* bd = spBoneData_create(boneName, parent);
		FREE(boneName);

		bd->x        = bin_read_float(s) * scale;
		bd->y        = bin_read_float(s) * scale;
		bd->scaleX   = bin_read_float(s);
		bd->scaleY   = bin_read_float(s);
		bd->rotation = bin_read_float(s);
		bd->length   = bin_read_float(s) * scale;
		bd->flipX    = bin_read_bool(s);
		bd->flipY    = bin_read_bool(s);
		bd->inheritScale    = bin_read_bool(s);
		bd->inheritRotation = bin_read_bool(s);

		if (nonessential) bin_read_int(s);

		sd->bones[i] = bd;
	}


	sd->ikConstraintsCount = bin_read_varint(s, 1);
	sd->ikConstraints = MALLOC(spIkConstraintData*, sd->ikConstraintsCount);
	for (i = 0; i < sd->ikConstraintsCount; ++i) {
		const char* ikName = bin_read_string(s);
		spIkConstraintData* ik = spIkConstraintData_create(ikName);
		int bi;
		FREE(ikName);

		ik->bonesCount = bin_read_varint(s, 1);
		ik->bones = MALLOC(spBoneData*, ik->bonesCount);
		for (bi = 0; bi < ik->bonesCount; ++bi)
			ik->bones[bi] = sd->bones[bin_read_varint(s, 1)];

		ik->target        = sd->bones[bin_read_varint(s, 1)];
		ik->mix           = bin_read_float(s);
		ik->bendDirection = bin_read_s8(s);

		sd->ikConstraints[i] = ik;
	}


	sd->slotsCount = bin_read_varint(s, 1);
	sd->slots = MALLOC(spSlotData*, sd->slotsCount);
	for (i = 0; i < sd->slotsCount; ++i) {
		const char* slotName = bin_read_string(s);
		spBoneData* boneRef = sd->bones[bin_read_varint(s, 1)];
		spSlotData* slot = spSlotData_create(slotName, boneRef);
		int color;
		FREE(slotName);

		color = bin_read_int(s);
		bin_decode_color(color, &slot->r, &slot->g, &slot->b, &slot->a);

		slot->attachmentName = bin_read_string(s);
		slot->additiveBlending = bin_read_bool(s);

		sd->slots[i] = slot;
	}


	sd->defaultSkin = bin_read_skin(self, s, "default", nonessential);
	if (self->attachmentLoader->error1) {
		spSkeletonData_dispose(sd);
		bin_set_error(self, self->attachmentLoader->error1, self->attachmentLoader->error2);
		return 0;
	}


	{
		int namedCount = bin_read_varint(s, 1);
		sd->skinsCount = namedCount + (sd->defaultSkin ? 1 : 0);
		sd->skins = MALLOC(spSkin*, sd->skinsCount);
		i = 0;
		if (sd->defaultSkin)
			sd->skins[i++] = sd->defaultSkin;
		for (; i < sd->skinsCount; ++i) {
			const char* skinName = bin_read_string(s);
			spSkin* skin = bin_read_skin(self, s, skinName, nonessential);
			FREE(skinName);
			if (self->attachmentLoader->error1) {
				spSkeletonData_dispose(sd);
				bin_set_error(self, self->attachmentLoader->error1, self->attachmentLoader->error2);
				return 0;
			}
			sd->skins[i] = skin;
		}
	}


	sd->eventsCount = bin_read_varint(s, 1);
	sd->events = MALLOC(spEventData*, sd->eventsCount);
	for (i = 0; i < sd->eventsCount; ++i) {
		const char* evName = bin_read_string(s);
		spEventData* evd = spEventData_create(evName);
		const char* strVal;
		FREE(evName);

		evd->intValue   = bin_read_varint(s, 0);
		evd->floatValue = bin_read_float(s);
		strVal = bin_read_string(s);
		if (strVal) MALLOC_STR(evd->stringValue, strVal);
		FREE(strVal);

		sd->events[i] = evd;
	}


	sd->animationsCount = bin_read_varint(s, 1);
	sd->animations = MALLOC(spAnimation*, sd->animationsCount);
	for (i = 0; i < sd->animationsCount; ++i) {
		const char* animName = bin_read_string(s);
		spAnimation* anim = bin_read_animation(self, animName, s, sd);
		FREE(animName);
		if (!anim) {
			sd->animationsCount = i;
			spSkeletonData_dispose(sd);
			return 0;
		}
		sd->animations[i] = anim;
	}

	return sd;
}
