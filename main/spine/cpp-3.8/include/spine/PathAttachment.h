
#ifndef Spine_PathAttachment_h
#define Spine_PathAttachment_h

#include <spine/VertexAttachment.h>

namespace spine {
	class SP_API PathAttachment : public VertexAttachment {
		friend class SkeletonBinary;
		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit PathAttachment(const String& name);

		Vector<float>& getLengths();
		bool isClosed();
		void setClosed(bool inValue);
		bool isConstantSpeed();
		void setConstantSpeed(bool inValue);

		virtual Attachment* copy();
	private:
		Vector<float> _lengths;
		bool _closed;
		bool _constantSpeed;
	};
}

#endif
