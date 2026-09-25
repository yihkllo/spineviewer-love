
#include <spine/AtlasAttachmentLoader.h>
#include <spine/extension.h>

spAttachment* _spAtlasAttachmentLoader_createAttachment (spAttachmentLoader* loader, spSkin* skin, spAttachmentType type,
		const char* name, const char* path) {
	spAtlasAttachmentLoader* self = SUB_CAST(spAtlasAttachmentLoader, loader);
	switch (type) {
	case SP_ATTACHMENT_REGION: {
		spRegionAttachment* attachment;
		spAtlasRegion* region = spAtlas_findRegion(self->atlas, path);
		if (!region) {
			_spAttachmentLoader_setError(loader, "Region not found: ", path);
			return 0;
		}
		attachment = spRegionAttachment_create(name);
		attachment->rendererObject = region;
		spRegionAttachment_setUVs(attachment, region->u, region->v, region->u2, region->v2, region->rotate);
		attachment->regionOffsetX = region->offsetX;
		attachment->regionOffsetY = region->offsetY;
		attachment->regionWidth = region->width;
		attachment->regionHeight = region->height;
		attachment->regionOriginalWidth = region->originalWidth;
		attachment->regionOriginalHeight = region->originalHeight;
		return SUPER(attachment);
	}
	case SP_ATTACHMENT_MESH:
	case SP_ATTACHMENT_LINKED_MESH: {
		spMeshAttachment* attachment;
		spAtlasRegion* region = spAtlas_findRegion(self->atlas, path);
		if (!region) {
			_spAttachmentLoader_setError(loader, "Region not found: ", path);
			return 0;
		}
		attachment = spMeshAttachment_create(name);
		attachment->rendererObject = region;
		attachment->regionU = region->u;
		attachment->regionV = region->v;
		attachment->regionU2 = region->u2;
		attachment->regionV2 = region->v2;
		attachment->regionRotate = region->rotate;
		attachment->regionOffsetX = region->offsetX;
		attachment->regionOffsetY = region->offsetY;
		attachment->regionWidth = region->width;
		attachment->regionHeight = region->height;
		attachment->regionOriginalWidth = region->originalWidth;
		attachment->regionOriginalHeight = region->originalHeight;
		return SUPER(SUPER(attachment));
	}
	case SP_ATTACHMENT_BOUNDING_BOX:
		return SUPER(SUPER(spBoundingBoxAttachment_create(name)));
	case SP_ATTACHMENT_PATH:
		return SUPER(SUPER(spPathAttachment_create(name)));
	default:
		_spAttachmentLoader_setUnknownTypeError(loader, type);
		return 0;
	}

	UNUSED(skin);
}

spAtlasAttachmentLoader* spAtlasAttachmentLoader_create (spAtlas* atlas) {
	spAtlasAttachmentLoader* self = NEW(spAtlasAttachmentLoader);
	_spAttachmentLoader_init(SUPER(self), _spAttachmentLoader_deinit, _spAtlasAttachmentLoader_createAttachment, 0, 0);
	self->atlas = atlas;
	return self;
}
