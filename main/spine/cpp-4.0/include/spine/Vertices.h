
#ifndef Spine_Vertices_h
#define Spine_Vertices_h

#include <spine/Vector.h>

namespace spine {
	class SP_API Vertices : public SpineObject {
	public:
		Vector <size_t> _bones;
		Vector<float> _vertices;
	};
}

#endif
