#ifndef Spine_RotateMode_h
#define Spine_RotateMode_h

#include <string.h>

namespace spine {

	enum RotateMode {
		RotateMode_Tangent = 0,
		RotateMode_Chain,

		RotateMode_ChainScale
	};

	inline RotateMode RotateMode_valueOf(const char *value) {
		if (strcmp(value, "tangent") == 0)
			return RotateMode_Tangent;
		else if (strcmp(value, "chain") == 0)
			return RotateMode_Chain;
		else if (strcmp(value, "chainScale") == 0)
			return RotateMode_ChainScale;
		else
			return RotateMode_Tangent;
	}
}

#endif
