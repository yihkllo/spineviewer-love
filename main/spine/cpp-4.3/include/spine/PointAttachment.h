#ifndef Spine_PointAttachment_h
#define Spine_PointAttachment_h

#include <spine/Attachment.h>
#include <spine/Color.h>

namespace spine {
	class Bone;
	class BonePose;

	class SP_API PointAttachment : public Attachment {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit PointAttachment(const String &name);

		float getX();

		void setX(float inValue);

		float getY();

		void setY(float inValue);

		float getRotation();

		void setRotation(float inValue);

		Color &getColor();

		void computeWorldPosition(BonePose &bone, float &ox, float &oy);

		float computeWorldRotation(BonePose &bone);

		virtual Attachment &copy() override;

	private:
		float _x, _y, _rotation;
		Color _color;
	};
}

#endif
