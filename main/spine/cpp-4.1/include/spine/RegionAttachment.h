
#ifndef Spine_RegionAttachment_h
#define Spine_RegionAttachment_h

#include <spine/Attachment.h>
#include <spine/Vector.h>
#include <spine/Color.h>
#include <spine/Sequence.h>
#include <spine/TextureRegion.h>

#include <spine/HasRendererObject.h>

#define NUM_UVS 8

namespace spine {
	class Bone;

	class SP_API RegionAttachment : public Attachment {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		friend class AtlasAttachmentLoader;

	RTTI_DECL

	public:
		explicit RegionAttachment(const String &name);

		virtual ~RegionAttachment();

		void updateRegion();

		void computeWorldVertices(Slot &slot, float *worldVertices, size_t offset, size_t stride = 2);

		void computeWorldVertices(Slot &slot, Vector<float> &worldVertices, size_t offset, size_t stride = 2);

		float getX();

		void setX(float inValue);

		float getY();

		void setY(float inValue);

		float getRotation();

		void setRotation(float inValue);

		float getScaleX();

		void setScaleX(float inValue);

		float getScaleY();

		void setScaleY(float inValue);

		float getWidth();

		void setWidth(float inValue);

		float getHeight();

		void setHeight(float inValue);

		Color &getColor();

		const String &getPath();

		void setPath(const String &inValue);

		TextureRegion *getRegion();

		void setRegion(TextureRegion *region);

		Sequence *getSequence();

		void setSequence(Sequence *sequence);

		Vector<float> &getOffset();

		Vector<float> &getUVs();

		virtual Attachment *copy();

	private:
		static const int BLX;
		static const int BLY;
		static const int ULX;
		static const int ULY;
		static const int URX;
		static const int URY;
		static const int BRX;
		static const int BRY;

		float _x, _y, _rotation, _scaleX, _scaleY, _width, _height;
		Vector<float> _vertexOffset;
		Vector<float> _uvs;
		String _path;
		Color _color;
		TextureRegion *_region;
		Sequence *_sequence;
	};
}

#endif
