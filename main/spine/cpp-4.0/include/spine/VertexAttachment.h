
#ifndef Spine_VertexAttachment_h
#define Spine_VertexAttachment_h

#include <spine/Attachment.h>

#include <spine/Vector.h>

namespace spine {
	class Slot;

	class SP_API VertexAttachment : public Attachment {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		friend class DeformTimeline;

	RTTI_DECL

	public:
		explicit VertexAttachment(const String &name);

		virtual ~VertexAttachment();

		void computeWorldVertices(Slot &slot, float *worldVertices);

		void computeWorldVertices(Slot &slot, Vector<float> &worldVertices);

		void computeWorldVertices(Slot &slot, size_t start, size_t count, float *worldVertices, size_t offset,
								  size_t stride = 2);

		void computeWorldVertices(Slot &slot, size_t start, size_t count, Vector<float> &worldVertices, size_t offset,
								  size_t stride = 2);

		int getId();

		Vector <size_t> &getBones();

		Vector<float> &getVertices();

		size_t getWorldVerticesLength();

		void setWorldVerticesLength(size_t inValue);

		VertexAttachment *getDeformAttachment();

		void setDeformAttachment(VertexAttachment *attachment);

		void copyTo(VertexAttachment *other);

	protected:
		Vector <size_t> _bones;
		Vector<float> _vertices;
		size_t _worldVerticesLength;
		VertexAttachment *_deformAttachment;

	private:
		const int _id;

		static int getNextID();
	};
}

#endif
