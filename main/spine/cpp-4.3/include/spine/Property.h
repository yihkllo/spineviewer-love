#ifndef Spine_Property_h
#define Spine_Property_h

namespace spine {
	typedef long long PropertyId;
	enum Property {
		Property_Rotate = 0,
		Property_X,
		Property_Y,
		Property_ScaleX,
		Property_ScaleY,
		Property_ShearX,
		Property_ShearY,
		Property_Inherit,
		Property_Rgb,
		Property_Alpha,
		Property_Rgb2,
		Property_Attachment,
		Property_Deform,
		Property_Event,
		Property_DrawOrder,
		Property_IkConstraint,
		Property_TransformConstraint,
		Property_PathConstraintPosition,
		Property_PathConstraintSpacing,
		Property_PathConstraintMix,
		Property_PhysicsConstraintInertia,
		Property_PhysicsConstraintStrength,
		Property_PhysicsConstraintDamping,
		Property_PhysicsConstraintMass,
		Property_PhysicsConstraintWind,
		Property_PhysicsConstraintGravity,
		Property_PhysicsConstraintMix,
		Property_PhysicsConstraintReset,
		Property_Sequence,
		Property_SliderTime,
		Property_SliderMix,
		Property_DrawOrderFolder
	};
}

#endif
