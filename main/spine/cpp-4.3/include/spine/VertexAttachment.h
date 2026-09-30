#ifndef Spine_VertexAttachment_h
#define Spine_VertexAttachment_h

#include <spine/Attachment.h>

#include <spine/Array.h>

namespace spine {
	class Slot;
	class Skeleton;

	class SP_API VertexAttachment : public Attachment {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		friend class DeformTimeline;

		RTTI_DECL

	public:
		explicit VertexAttachment(const String &name);

		virtual ~VertexAttachment();

		virtual void computeWorldVertices(Skeleton &skeleton, Slot &slot, size_t start, size_t count, float *worldVertices, size_t offset,
										  size_t stride = 2);

		virtual void computeWorldVertices(Skeleton &skeleton, Slot &slot, size_t start, size_t count, Array<float> &worldVertices, size_t offset,
										  size_t stride = 2);

		int getId();

		Array<int> &getBones();

		void setBones(Array<int> &bones);

		Array<float> &getVertices();

		void setVertices(Array<float> &vertices);

		size_t getWorldVerticesLength();

		void setWorldVerticesLength(size_t inValue);

		Attachment *getTimelineAttachment();

		void setTimelineAttachment(Attachment *attachment);

		void copyTo(VertexAttachment &other);

	protected:
		Array<int> _bones;
		Array<float> _vertices;
		size_t _worldVerticesLength;

	private:
		const int _id;

		static int getNextID();
	};
}

#endif
