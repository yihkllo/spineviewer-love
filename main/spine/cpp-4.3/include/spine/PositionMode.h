#ifndef Spine_PositionMode_h
#define Spine_PositionMode_h

#include <string.h>

namespace spine {

	enum PositionMode {
		PositionMode_Fixed = 0,
		PositionMode_Percent
	};

	inline PositionMode PositionMode_valueOf(const char *value) {
		if (strcmp(value, "fixed") == 0)
			return PositionMode_Fixed;
		else if (strcmp(value, "percent") == 0)
			return PositionMode_Percent;
		else
			return PositionMode_Percent;
	}
}

#endif
