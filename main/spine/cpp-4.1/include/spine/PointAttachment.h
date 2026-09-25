
#ifndef Spine_PointAttachment_h
#define Spine_PointAttachment_h

#include <spine/Attachment.h>
#include <spine/Color.h>

namespace spine {
	class Bone;

	class SP_API PointAttachment : public Attachment {
		friend class SkeletonBinary;

		friend class SkeletonJson;

	RTTI_DECL

	public:
		explicit PointAttachment(const String &name);

		void computeWorldPosition(Bone &bone, float &ox, float &oy);

		float computeWorldRotation(Bone &bone);

		float getX();

		void setX(float inValue);

		float getY();

		void setY(float inValue);

		float getRotation();

		void setRotation(float inValue);

		Color &getColor();

		virtual Attachment *copy();

	private:
		float _x, _y, _rotation;
		Color _color;
	};
}

#endif
