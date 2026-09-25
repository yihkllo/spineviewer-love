
#ifndef Spine_MeshAttachment_h
#define Spine_MeshAttachment_h

#include <spine/VertexAttachment.h>
#include <spine/Vector.h>
#include <spine/Color.h>
#include <spine/HasRendererObject.h>

namespace spine {
	class SP_API MeshAttachment : public VertexAttachment, public HasRendererObject {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		friend class AtlasAttachmentLoader;

	RTTI_DECL

	public:
		explicit MeshAttachment(const String &name);

		virtual ~MeshAttachment();

		void updateUVs();

		int getHullLength();

		void setHullLength(int inValue);

		Vector<float> &getRegionUVs();

		Vector<float> &getUVs();

		Vector<unsigned short> &getTriangles();

		Color &getColor();

		const String &getPath();

		void setPath(const String &inValue);

		float getRegionU();

		void setRegionU(float inValue);

		float getRegionV();

		void setRegionV(float inValue);

		float getRegionU2();

		void setRegionU2(float inValue);

		float getRegionV2();

		void setRegionV2(float inValue);

		int getRegionDegrees();

		void setRegionDegrees(int inValue);

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

		MeshAttachment *getParentMesh();

		void setParentMesh(MeshAttachment *inValue);

		Vector<unsigned short> &getEdges();

		float getWidth();

		void setWidth(float inValue);

		float getHeight();

		void setHeight(float inValue);

		virtual Attachment *copy();

		MeshAttachment *newLinkedMesh();

	private:
		float _regionOffsetX, _regionOffsetY, _regionWidth, _regionHeight, _regionOriginalWidth, _regionOriginalHeight;
		MeshAttachment *_parentMesh;
		Vector<float> _uvs;
		Vector<float> _regionUVs;
		Vector<unsigned short> _triangles;
		Vector<unsigned short> _edges;
		String _path;
		float _regionU;
		float _regionV;
		float _regionU2;
		float _regionV2;
		float _width;
		float _height;
		Color _color;
		int _hullLength;
		int _regionDegrees;
	};
}

#endif
