
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

	bool loadSequence(Atlas *atlas, const String &basePath, Sequence *sequence) {
		Vector<TextureRegion *> &regions = sequence->getRegions();
		for (int i = 0, n = (int) regions.size(); i < n; i++) {
			String path = sequence->getPath(basePath, i);
			regions[i] = atlas->findRegion(path);
			if (!regions[i]) return false;
		}
		return true;
	}

	RegionAttachment *AtlasAttachmentLoader::newRegionAttachment(Skin &skin, const String &name, const String &path, Sequence *sequence) {
		SP_UNUSED(skin);
		RegionAttachment *attachment = new (__FILE__, __LINE__) RegionAttachment(name);
		if (sequence) {
			if (!loadSequence(_atlas, path, sequence)) return NULL;
		} else {
			AtlasRegion *region = findRegion(path);
			if (!region) return NULL;
			attachment->setRegion(region);
		}
		return attachment;
	}

	MeshAttachment *AtlasAttachmentLoader::newMeshAttachment(Skin &skin, const String &name, const String &path, Sequence *sequence) {
		SP_UNUSED(skin);
		MeshAttachment *attachment = new (__FILE__, __LINE__) MeshAttachment(name);

		if (sequence) {
			if (!loadSequence(_atlas, path, sequence)) return NULL;
		} else {
			AtlasRegion *region = findRegion(path);
			if (!region) return NULL;
			attachment->setRegion(region);
		}
		return attachment;
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
