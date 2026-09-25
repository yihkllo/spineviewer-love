
#ifndef Spine_Property_h
#define Spine_Property_h

namespace spine {
	typedef long long PropertyId;
	enum Property {
		Property_Rotate = 1 << 0,
		Property_X = 1 << 1,
		Property_Y = 1 << 2,
		Property_ScaleX = 1 << 3,
		Property_ScaleY = 1 << 4,
		Property_ShearX = 1 << 5,
		Property_ShearY = 1 << 6,
        Property_Inherit = 1 << 7,
		Property_Rgb = 1 << 8,
		Property_Alpha = 1 << 9,
		Property_Rgb2 = 1 << 10,
		Property_Attachment = 1 << 11,
		Property_Deform = 1 << 12,
		Property_Event = 1 << 13,
		Property_DrawOrder = 1 << 14,
		Property_IkConstraint = 1 << 15,
		Property_TransformConstraint = 1 << 16,
		Property_PathConstraintPosition = 1 << 17,
		Property_PathConstraintSpacing = 1 << 18,
		Property_PathConstraintMix = 1 << 19,
        Property_PhysicsConstraintInertia = 1 << 20,
        Property_PhysicsConstraintStrength = 1 << 21,
        Property_PhysicsConstraintDamping = 1 << 22,
        Property_PhysicsConstraintMass = 1 << 23,
        Property_PhysicsConstraintWind = 1 << 24,
        Property_PhysicsConstraintGravity = 1 << 25,
        Property_PhysicsConstraintMix = 1 << 26,
        Property_PhysicsConstraintReset = 1 << 27,
		Property_Sequence = 1 << 28
	};
}

#endif
