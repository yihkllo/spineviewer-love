#ifndef Spine_Pose_h
#define Spine_Pose_h

#include <spine/SpineObject.h>

namespace spine {

	template<class P>
	class SP_API Pose : public SpineObject {
	public:
		Pose() {};
		virtual ~Pose() {};

		virtual void set(P &pose) = 0;
	};
}

#endif
