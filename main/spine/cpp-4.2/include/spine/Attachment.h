
#ifndef Spine_Attachment_h
#define Spine_Attachment_h

#include <spine/RTTI.h>
#include <spine/SpineObject.h>
#include <spine/SpineString.h>

namespace spine {
	class SP_API Attachment : public SpineObject {
	RTTI_DECL

	public:
		explicit Attachment(const String &name);

		virtual ~Attachment();

		const String &getName() const;

		virtual Attachment *copy() = 0;

		int getRefCount();

		void reference();

		void dereference();

	private:
		const String _name;
		int _refCount;
	};
}

#endif
