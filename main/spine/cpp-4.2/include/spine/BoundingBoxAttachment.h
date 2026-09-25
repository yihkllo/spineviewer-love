
#ifndef Spine_BoundingBoxAttachment_h
#define Spine_BoundingBoxAttachment_h

#include <spine/VertexAttachment.h>
#include <spine/Color.h>
#include <spine/SpineObject.h>

namespace spine {
	class SP_API BoundingBoxAttachment : public VertexAttachment {
	RTTI_DECL

	public:
		explicit BoundingBoxAttachment(const String &name);

		Color &getColor();

		virtual Attachment *copy();

	private:
		Color _color;
	};
}

#endif
