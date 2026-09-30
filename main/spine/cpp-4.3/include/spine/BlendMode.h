#ifndef Spine_BlendMode_h
#define Spine_BlendMode_h

#include <string.h>

namespace spine {
	enum BlendMode {
		BlendMode_Normal = 0,
		BlendMode_Additive,
		BlendMode_Multiply,
		BlendMode_Screen
	};

	inline BlendMode BlendMode_valueOf(const char *value) {
		if (strcmp(value, "normal") == 0)
			return BlendMode_Normal;
		else if (strcmp(value, "additive") == 0)
			return BlendMode_Additive;
		else if (strcmp(value, "multiply") == 0)
			return BlendMode_Multiply;
		else if (strcmp(value, "screen") == 0)
			return BlendMode_Screen;
		else
			return BlendMode_Normal;
	}
}

#endif
