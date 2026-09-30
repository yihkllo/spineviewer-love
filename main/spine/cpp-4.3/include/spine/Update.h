#ifndef Spine_Update_h
#define Spine_Update_h

#include <spine/dll.h>
#include <spine/Physics.h>
#include <spine/RTTI.h>
#include <spine/SpineObject.h>

namespace spine {
	class Skeleton;

	class SP_API Update : public SpineObject {
		RTTI_DECL_NOPARENT
	public:
		Update();
		virtual ~Update();

		virtual void update(Skeleton &skeleton, Physics physics) = 0;
	};
}

#endif
