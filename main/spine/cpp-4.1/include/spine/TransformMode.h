
#ifndef Spine_TransformMode_h
#define Spine_TransformMode_h

namespace spine {
	enum TransformMode {
		TransformMode_Normal = 0,
		TransformMode_OnlyTranslation,
		TransformMode_NoRotationOrReflection,
		TransformMode_NoScale,
		TransformMode_NoScaleOrReflection
	};
}

#endif
