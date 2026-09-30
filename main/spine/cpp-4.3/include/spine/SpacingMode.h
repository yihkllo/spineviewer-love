#ifndef Spine_SpacingMode_h
#define Spine_SpacingMode_h

#include <string.h>

namespace spine {

	enum SpacingMode {
		SpacingMode_Length = 0,
		SpacingMode_Fixed,
		SpacingMode_Percent,
		SpacingMode_Proportional
	};

	inline SpacingMode SpacingMode_valueOf(const char *value) {
		if (strcmp(value, "length") == 0)
			return SpacingMode_Length;
		else if (strcmp(value, "fixed") == 0)
			return SpacingMode_Fixed;
		else if (strcmp(value, "percent") == 0)
			return SpacingMode_Percent;
		else if (strcmp(value, "proportional") == 0)
			return SpacingMode_Proportional;
		else
			return SpacingMode_Length;
	}
}

#endif
