
#ifndef Spine_Updatable_h
#define Spine_Updatable_h

#include <spine/RTTI.h>
#include <spine/SpineObject.h>

namespace spine {
	class SP_API Updatable : public SpineObject {
	RTTI_DECL

	public:
		Updatable();

		virtual ~Updatable();

		virtual void update() = 0;

		virtual bool isActive() = 0;

		virtual void setActive(bool inValue) = 0;
	};
}

#endif
