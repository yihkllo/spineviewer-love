
#ifndef Spine_AtlasAttachmentLoader_h
#define Spine_AtlasAttachmentLoader_h

#include <spine/AttachmentLoader.h>
#include <spine/Vector.h>
#include <spine/SpineString.h>


namespace spine {
	class Atlas;

	class AtlasRegion;

	class SP_API AtlasAttachmentLoader : public AttachmentLoader {
	public:
	RTTI_DECL

		explicit AtlasAttachmentLoader(Atlas *atlas);

		virtual RegionAttachment *newRegionAttachment(Skin &skin, const String &name, const String &path);

		virtual MeshAttachment *newMeshAttachment(Skin &skin, const String &name, const String &path);

		virtual BoundingBoxAttachment *newBoundingBoxAttachment(Skin &skin, const String &name);

		virtual PathAttachment *newPathAttachment(Skin &skin, const String &name);

		virtual PointAttachment *newPointAttachment(Skin &skin, const String &name);

		virtual ClippingAttachment *newClippingAttachment(Skin &skin, const String &name);

		virtual void configureAttachment(Attachment *attachment);

		AtlasRegion *findRegion(const String &name);

	private:
		Atlas *_atlas;
	};
}

#endif
