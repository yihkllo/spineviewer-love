
#ifndef Spine_TimelineType_h
#define Spine_TimelineType_h

namespace spine {
enum TimelineType {
	TimelineType_Rotate = 0,
	TimelineType_Translate,
	TimelineType_Scale,
	TimelineType_Shear,
	TimelineType_Attachment,
	TimelineType_Color,
	TimelineType_Deform,
	TimelineType_Event,
	TimelineType_DrawOrder,
	TimelineType_IkConstraint,
	TimelineType_TransformConstraint,
	TimelineType_PathConstraintPosition,
	TimelineType_PathConstraintSpacing,
	TimelineType_PathConstraintMix,
	TimelineType_TwoColor
};
}

#endif
