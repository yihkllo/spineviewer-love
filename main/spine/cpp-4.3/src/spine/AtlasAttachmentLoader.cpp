#include <spine/AtlasAttachmentLoader.h>
#include <spine/BoundingBoxAttachment.h>
#include <spine/ClippingAttachment.h>
#include <spine/MeshAttachment.h>
#include <spine/PathAttachment.h>
#include <spine/PointAttachment.h>
#include <spine/RegionAttachment.h>
#include <spine/Skin.h>

#include <spine/Atlas.h>

using namespace spine;

AtlasAttachmentLoader::AtlasAttachmentLoader(Atlas &atlas) : AttachmentLoader(), _atlas(&atlas) {
}

static void findRegions(Atlas *atlas, AtlasAttachmentLoader *loader, const String &name, const String &basePath, Sequence *sequence) {
	Array<TextureRegion *> &regions = sequence->getRegions();
	for (int i = 0, n = (int) regions.size(); i < n; i++) {
		String path = sequence->getPath(basePath, i);
		regions[i] = loader->findRegion(path);
	}
}

RegionAttachment *AtlasAttachmentLoader::newRegionAttachment(Skin &skin, const String &placeholder, const String &name, const String &path,
															 Sequence *sequence) {
	SP_UNUSED(skin);
	SP_UNUSED(placeholder);
	findRegions(_atlas, this, name, path, sequence);
	return new (__FILE__, __LINE__) RegionAttachment(name, sequence);
}

MeshAttachment *AtlasAttachmentLoader::newMeshAttachment(Skin &skin, const String &placeholder, const String &name, const String &path,
														 Sequence *sequence) {
	SP_UNUSED(skin);
	SP_UNUSED(placeholder);
	findRegions(_atlas, this, name, path, sequence);
	return new (__FILE__, __LINE__) MeshAttachment(name, sequence);
}

BoundingBoxAttachment *AtlasAttachmentLoader::newBoundingBoxAttachment(Skin &skin, const String &placeholder, const String &name) {
	SP_UNUSED(skin);
	SP_UNUSED(placeholder);
	return new (__FILE__, __LINE__) BoundingBoxAttachment(name);
}

PathAttachment *AtlasAttachmentLoader::newPathAttachment(Skin &skin, const String &placeholder, const String &name) {
	SP_UNUSED(skin);
	SP_UNUSED(placeholder);
	return new (__FILE__, __LINE__) PathAttachment(name);
}

PointAttachment *AtlasAttachmentLoader::newPointAttachment(Skin &skin, const String &placeholder, const String &name) {
	SP_UNUSED(skin);
	SP_UNUSED(placeholder);
	return new (__FILE__, __LINE__) PointAttachment(name);
}

ClippingAttachment *AtlasAttachmentLoader::newClippingAttachment(Skin &skin, const String &placeholder, const String &name) {
	SP_UNUSED(skin);
	SP_UNUSED(placeholder);
	return new (__FILE__, __LINE__) ClippingAttachment(name);
}

AtlasRegion *AtlasAttachmentLoader::findRegion(const String &name) {
	return _atlas->findRegion(name);
}
