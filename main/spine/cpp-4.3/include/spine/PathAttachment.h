#ifndef Spine_PathAttachment_h
#define Spine_PathAttachment_h

#include <spine/VertexAttachment.h>
#include <spine/Color.h>

namespace spine {
	class SP_API PathAttachment : public VertexAttachment {
		friend class SkeletonBinary;

		friend class SkeletonJson;

		RTTI_DECL

	public:
		explicit PathAttachment(const String &name);

		Array<float> &getLengths();

		void setLengths(Array<float> &inValue);

		bool getClosed();

		void setClosed(bool inValue);

		bool getConstantSpeed();

		void setConstantSpeed(bool inValue);

		Color &getColor();

		virtual Attachment &copy() override;

	private:
		Array<float> _lengths;
		bool _closed;
		bool _constantSpeed;
		Color _color;
	};
}

#endif
