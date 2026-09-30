#ifndef Spine_MeshAttachment_h
#define Spine_MeshAttachment_h

#include <spine/Array.h>
#include <spine/Color.h>
#include <spine/HasRendererObject.h>
#include <spine/Sequence.h>
#include <spine/TextureRegion.h>
#include <spine/VertexAttachment.h>

namespace spine {

	class SP_API MeshAttachment : public VertexAttachment {
		friend class SkeletonBinary;
		friend class SkeletonJson;
		friend class AtlasAttachmentLoader;

		RTTI_DECL

	public:
		explicit MeshAttachment(const String &name, Sequence *sequence);

		virtual ~MeshAttachment();

		using VertexAttachment::computeWorldVertices;

		virtual void computeWorldVertices(Skeleton &skeleton, Slot &slot, size_t start, size_t count, float *worldVertices, size_t offset,
										  size_t stride = 2) override;

		Array<float> &getRegionUVs();
		void setRegionUVs(Array<float> &inValue);

		Array<unsigned short> &getTriangles();
		void setTriangles(Array<unsigned short> &inValue);

		int getHullLength();
		void setHullLength(int inValue);

		Sequence &getSequence();

		void updateSequence();

		const String &getPath();
		void setPath(const String &inValue);

		Color &getColor();

		MeshAttachment *getSourceMesh();
		void setSourceMesh(MeshAttachment *inValue);

		Array<unsigned short> &getEdges();
		void setEdges(Array<unsigned short> &inValue);

		float getWidth();
		void setWidth(float inValue);

		float getHeight();
		void setHeight(float inValue);

		virtual Attachment &copy() override;

		MeshAttachment &newLinkedMesh();

		static void computeUVs(TextureRegion *region, Array<float> &regionUVs, Array<float> &uvs);

	private:
		Sequence *_sequence;
		Array<float> _regionUVs;
		Array<unsigned short> _triangles;
		int _hullLength;
		String _path;
		Color _color;
		MeshAttachment *_sourceMesh;

		Array<unsigned short> _edges;
		float _width, _height;
	};
}

#endif
