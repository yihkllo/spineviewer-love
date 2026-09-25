
#ifndef Spine_MeshAttachment_h
#define Spine_MeshAttachment_h

#include <spine/VertexAttachment.h>
#include <spine/TextureRegion.h>
#include <spine/Sequence.h>
#include <spine/Vector.h>
#include <spine/Color.h>
#include <spine/HasRendererObject.h>

namespace spine {
	class SP_API MeshAttachment : public VertexAttachment {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		friend class AtlasAttachmentLoader;

	RTTI_DECL

	public:
		explicit MeshAttachment(const String &name);

		virtual ~MeshAttachment();

		using VertexAttachment::computeWorldVertices;

		virtual void computeWorldVertices(Slot &slot, size_t start, size_t count, float *worldVertices, size_t offset,
		size_t stride = 2);

		void updateRegion();

		int getHullLength();

		void setHullLength(int inValue);

		Vector<float> &getRegionUVs();

		Vector<float> &getUVs();

		Vector<unsigned short> &getTriangles();

		Color &getColor();

		const String &getPath();

		void setPath(const String &inValue);

		TextureRegion *getRegion();

		void setRegion(TextureRegion *region);

		Sequence *getSequence();

		void setSequence(Sequence *sequence);

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
		MeshAttachment *_parentMesh;
		Vector<float> _uvs;
		Vector<float> _regionUVs;
		Vector<unsigned short> _triangles;
		Vector<unsigned short> _edges;
		String _path;
		Color _color;
		int _hullLength;
		int _width, _height;
		TextureRegion *_region;
		Sequence *_sequence;
	};
}

#endif
