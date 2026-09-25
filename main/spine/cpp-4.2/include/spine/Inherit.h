
#ifndef Spine_TransformMode_h
#define Spine_TransformMode_h

namespace spine {
	enum Inherit {
		Inherit_Normal = 0,
		Inherit_OnlyTranslation,
		Inherit_NoRotationOrReflection,
		Inherit_NoScale,
		Inherit_NoScaleOrReflection
	};
}

#endif
