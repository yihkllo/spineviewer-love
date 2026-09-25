
#ifndef Spine_BoundingBoxAttachment_h
#define Spine_BoundingBoxAttachment_h

#include <spine/VertexAttachment.h>
#include <spine/SpineObject.h>

namespace spine {
	class SP_API BoundingBoxAttachment : public VertexAttachment {
		RTTI_DECL

		explicit BoundingBoxAttachment(const String& name);

		virtual Attachment* copy();
	};
}

#endif
