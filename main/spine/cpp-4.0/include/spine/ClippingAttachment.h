
#ifndef Spine_ClippingAttachment_h
#define Spine_ClippingAttachment_h

#include <spine/VertexAttachment.h>
#include <spine/Color.h>

namespace spine {
	class SlotData;

	class SP_API ClippingAttachment : public VertexAttachment {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		friend class SkeletonClipping;

	RTTI_DECL

	public:
		explicit ClippingAttachment(const String &name);

		SlotData *getEndSlot();

		void setEndSlot(SlotData *inValue);

		Color &getColor();

		virtual Attachment *copy();

	private:
		SlotData *_endSlot;
		Color _color;
	};
}

#endif
