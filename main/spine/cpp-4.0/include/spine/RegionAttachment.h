
#ifndef Spine_RegionAttachment_h
#define Spine_RegionAttachment_h

#include <spine/Attachment.h>
#include <spine/Vector.h>
#include <spine/Color.h>

#include <spine/HasRendererObject.h>

#define NUM_UVS 8

namespace spine {
	class Bone;

	class SP_API RegionAttachment : public Attachment, public HasRendererObject {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		friend class AtlasAttachmentLoader;

	RTTI_DECL

	public:
		explicit RegionAttachment(const String &name);

		void updateOffset();

		void setUVs(float u, float v, float u2, float v2, float degrees);

		void computeWorldVertices(Bone &bone, float *worldVertices, size_t offset, size_t stride = 2);

		void computeWorldVertices(Bone &bone, Vector<float> &worldVertices, size_t offset, size_t stride = 2);

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

		float getRegionOffsetX();

		void setRegionOffsetX(float inValue);

		float getRegionOffsetY();

		void setRegionOffsetY(float inValue);

		float getRegionWidth();

		void setRegionWidth(float inValue);

		float getRegionHeight();

		void setRegionHeight(float inValue);

		float getRegionOriginalWidth();

		void setRegionOriginalWidth(float inValue);

		float getRegionOriginalHeight();

		void setRegionOriginalHeight(float inValue);

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
		float _regionOffsetX, _regionOffsetY, _regionWidth, _regionHeight, _regionOriginalWidth, _regionOriginalHeight;
		Vector<float> _vertexOffset;
		Vector<float> _uvs;
		String _path;
		float _regionU;
		float _regionV;
		float _regionU2;
		float _regionV2;
		Color _color;
	};
}

#endif
