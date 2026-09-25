
#ifdef SPINE_UE4
#include "SpinePluginPrivatePCH.h"
#endif

#include <spine/AtlasAttachmentLoader.h>
#include <spine/BoundingBoxAttachment.h>
#include <spine/ClippingAttachment.h>
#include <spine/MeshAttachment.h>
#include <spine/PathAttachment.h>
#include <spine/PointAttachment.h>
#include <spine/RegionAttachment.h>
#include <spine/Skin.h>

#include <spine/Atlas.h>

namespace spine {
	RTTI_IMPL(AtlasAttachmentLoader, AttachmentLoader)

	AtlasAttachmentLoader::AtlasAttachmentLoader(Atlas *atlas) : AttachmentLoader(), _atlas(atlas) {
	}

	RegionAttachment *AtlasAttachmentLoader::newRegionAttachment(Skin &skin, const String &name, const String &path) {
		SP_UNUSED(skin);

		AtlasRegion *regionP = findRegion(path);
		if (!regionP) return NULL;

		AtlasRegion &region = *regionP;

		RegionAttachment *attachmentP = new (__FILE__, __LINE__) RegionAttachment(name);

		RegionAttachment &attachment = *attachmentP;
		attachment.setRendererObject(regionP);
		attachment.setUVs(region.u, region.v, region.u2, region.v2, region.degrees);
		attachment._regionOffsetX = region.offsetX;
		attachment._regionOffsetY = region.offsetY;
		attachment._regionWidth = (float) region.width;
		attachment._regionHeight = (float) region.height;
		attachment._regionOriginalWidth = (float) region.originalWidth;
		attachment._regionOriginalHeight = (float) region.originalHeight;
		return attachmentP;
	}

	MeshAttachment *AtlasAttachmentLoader::newMeshAttachment(Skin &skin, const String &name, const String &path) {
		SP_UNUSED(skin);

		AtlasRegion *regionP = findRegion(path);
		if (!regionP) return NULL;

		AtlasRegion &region = *regionP;

		MeshAttachment *attachmentP = new (__FILE__, __LINE__) MeshAttachment(name);

		MeshAttachment &attachment = *attachmentP;
		attachment.setRendererObject(regionP);
		attachment._regionU = region.u;
		attachment._regionV = region.v;
		attachment._regionU2 = region.u2;
		attachment._regionV2 = region.v2;
		attachment._regionDegrees = region.degrees;
		attachment._regionOffsetX = region.offsetX;
		attachment._regionOffsetY = region.offsetY;
		attachment._regionWidth = (float) region.width;
		attachment._regionHeight = (float) region.height;
		attachment._regionOriginalWidth = (float) region.originalWidth;
		attachment._regionOriginalHeight = (float) region.originalHeight;

		return attachmentP;
	}

	BoundingBoxAttachment *AtlasAttachmentLoader::newBoundingBoxAttachment(Skin &skin, const String &name) {
		SP_UNUSED(skin);
		return new (__FILE__, __LINE__) BoundingBoxAttachment(name);
	}

	PathAttachment *AtlasAttachmentLoader::newPathAttachment(Skin &skin, const String &name) {
		SP_UNUSED(skin);
		return new (__FILE__, __LINE__) PathAttachment(name);
	}

	PointAttachment *AtlasAttachmentLoader::newPointAttachment(Skin &skin, const String &name) {
		SP_UNUSED(skin);
		return new (__FILE__, __LINE__) PointAttachment(name);
	}

	ClippingAttachment *AtlasAttachmentLoader::newClippingAttachment(Skin &skin, const String &name) {
		SP_UNUSED(skin);
		return new (__FILE__, __LINE__) ClippingAttachment(name);
	}

	void AtlasAttachmentLoader::configureAttachment(Attachment *attachment) {
		SP_UNUSED(attachment);
	}

	AtlasRegion *AtlasAttachmentLoader::findRegion(const String &name) {
		return _atlas->findRegion(name);
	}

}
