#ifndef Spine_TransformMode_h
#define Spine_TransformMode_h

#include <string.h>

namespace spine {

	enum Inherit {
		Inherit_Normal = 0,
		Inherit_OnlyTranslation,
		Inherit_NoRotationOrReflection,
		Inherit_NoScale,
		Inherit_NoScaleOrReflection
	};

	inline Inherit Inherit_valueOf(const char *value) {
		if (strcmp(value, "normal") == 0)
			return Inherit_Normal;
		else if (strcmp(value, "onlyTranslation") == 0)
			return Inherit_OnlyTranslation;
		else if (strcmp(value, "noRotationOrReflection") == 0)
			return Inherit_NoRotationOrReflection;
		else if (strcmp(value, "noScale") == 0)
			return Inherit_NoScale;
		else if (strcmp(value, "noScaleOrReflection") == 0)
			return Inherit_NoScaleOrReflection;
		else
			return Inherit_Normal;
	}
}

#endif
