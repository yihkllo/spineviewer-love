#ifndef Spine_AtlasAttachmentLoader_h
#define Spine_AtlasAttachmentLoader_h

#include <spine/AttachmentLoader.h>
#include <spine/Array.h>
#include <spine/SpineString.h>

namespace spine {
	class Atlas;

	class AtlasRegion;

	class SP_API AtlasAttachmentLoader : public AttachmentLoader {
	public:
		explicit AtlasAttachmentLoader(Atlas &atlas);

		virtual RegionAttachment *newRegionAttachment(Skin &skin, const String &placeholder, const String &name, const String &path,
													  Sequence *sequence);

		virtual MeshAttachment *newMeshAttachment(Skin &skin, const String &placeholder, const String &name, const String &path, Sequence *sequence);

		virtual BoundingBoxAttachment *newBoundingBoxAttachment(Skin &skin, const String &placeholder, const String &name);

		virtual PathAttachment *newPathAttachment(Skin &skin, const String &placeholder, const String &name);

		virtual PointAttachment *newPointAttachment(Skin &skin, const String &placeholder, const String &name);

		virtual ClippingAttachment *newClippingAttachment(Skin &skin, const String &placeholder, const String &name);

		AtlasRegion *findRegion(const String &name);

	private:
		Atlas *_atlas;
	};
}

#endif
