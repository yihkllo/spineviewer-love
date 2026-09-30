#ifndef Spine_RegionAttachment_h
#define Spine_RegionAttachment_h

#include <spine/Attachment.h>
#include <spine/Array.h>
#include <spine/Color.h>
#include <spine/HasRendererObject.h>
#include <spine/Sequence.h>
#include <spine/TextureRegion.h>

namespace spine {
	class Slot;
	class SlotPose;

	class SP_API RegionAttachment : public Attachment {
		friend class SkeletonBinary;
		friend class SkeletonJson;
		friend class AtlasAttachmentLoader;

		RTTI_DECL

	public:
		explicit RegionAttachment(const String &name, Sequence *sequence);

		virtual ~RegionAttachment();

		void computeWorldVertices(Slot &slot, float *vertexOffsets, float *worldVertices, size_t offset, size_t stride = 2);

		void computeWorldVertices(Slot &slot, Array<float> &vertexOffsets, Array<float> &worldVertices, size_t offset, size_t stride = 2);

		Array<float> &getOffsets(SlotPose &pose);

		float getX();
		void setX(float inValue);

		float getY();
		void setY(float inValue);

		float getScaleX();
		void setScaleX(float inValue);

		float getScaleY();
		void setScaleY(float inValue);

		float getRotation();
		void setRotation(float inValue);

		float getWidth();
		void setWidth(float inValue);

		float getHeight();
		void setHeight(float inValue);

		Sequence &getSequence();

		void updateSequence();

		const String &getPath();
		void setPath(const String &inValue);

		Color &getColor();

		virtual Attachment &copy() override;

		static void computeUVs(TextureRegion *region, float x, float y, float scaleX, float scaleY, float rotation, float width, float height,
							   Array<float> &offset, Array<float> &uvs);

	private:
		static const int BLX;
		static const int BLY;
		static const int ULX;
		static const int ULY;
		static const int URX;
		static const int URY;
		static const int BRX;
		static const int BRY;

		Sequence *_sequence;
		float _x, _y, _scaleX, _scaleY, _rotation, _width, _height;
		String _path;
		Color _color;
	};
}

#endif
