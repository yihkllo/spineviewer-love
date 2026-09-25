
#ifndef Spine_AttachmentLoader_h
#define Spine_AttachmentLoader_h

#include <spine/RTTI.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>

namespace spine {
	class Skin;

	class Attachment;

	class RegionAttachment;

	class MeshAttachment;

	class BoundingBoxAttachment;

	class PathAttachment;

	class PointAttachment;

	class ClippingAttachment;

	class SP_API AttachmentLoader : public SpineObject {
	public:
	RTTI_DECL

		AttachmentLoader();

		virtual ~AttachmentLoader();

		virtual RegionAttachment *newRegionAttachment(Skin &skin, const String &name, const String &path) = 0;

		virtual MeshAttachment *newMeshAttachment(Skin &skin, const String &name, const String &path) = 0;

		virtual BoundingBoxAttachment *newBoundingBoxAttachment(Skin &skin, const String &name) = 0;

		virtual PathAttachment *newPathAttachment(Skin &skin, const String &name) = 0;

		virtual PointAttachment *newPointAttachment(Skin &skin, const String &name) = 0;

		virtual ClippingAttachment *newClippingAttachment(Skin &skin, const String &name) = 0;

		virtual void configureAttachment(Attachment *attachment) = 0;
	};
}

#endif
