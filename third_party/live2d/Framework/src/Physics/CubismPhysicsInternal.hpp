


#pragma once

#include "Model/CubismModel.hpp"
#include "Math/CubismVector2.hpp"
#include "Id/CubismId.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

enum CubismPhysicsTargetType
{
    CubismPhysicsTargetType_Parameter,
};

enum CubismPhysicsSource
{
    CubismPhysicsSource_X,
    CubismPhysicsSource_Y,
    CubismPhysicsSource_Angle,
};

struct PhysicsJsonEffectiveForces
{
    CubismVector2 Gravity;
    CubismVector2 Wind;
};

struct CubismPhysicsParameter
{
    CubismIdHandle Id;
    CubismPhysicsTargetType TargetType;
};

struct CubismPhysicsNormalization
{
    csmFloat32 Minimum;
    csmFloat32 Maximum;
    csmFloat32 Default;
};

struct CubismPhysicsParticle
{
    CubismVector2 InitialPosition;
    csmFloat32 Mobility;
    csmFloat32 Delay;
    csmFloat32 Acceleration;
    csmFloat32 Radius;
    CubismVector2 Position;
    CubismVector2 LastPosition;
    CubismVector2 LastGravity;
    CubismVector2 Force;
    CubismVector2 Velocity;
};

struct CubismPhysicsSubRig
{
    csmInt32 InputCount;
    csmInt32 OutputCount;
    csmInt32 ParticleCount;
    csmInt32 BaseInputIndex;
    csmInt32 BaseOutputIndex;
    csmInt32 BaseParticleIndex;
    CubismPhysicsNormalization NormalizationPosition;
    CubismPhysicsNormalization NormalizationAngle;
};

typedef void (*NormalizedPhysicsParameterValueGetter)(
    CubismVector2* targetTranslation,
    csmFloat32* targetAngle,
    csmFloat32 value,
    csmFloat32 parameterMinimumValue,
    csmFloat32 parameterMaximumValue,
    csmFloat32 parameterDefaultValue,
    CubismPhysicsNormalization* normalizationPosition,
    CubismPhysicsNormalization* normalizationAngle,
    csmInt32 isInverted,
    csmFloat32 weight
);

typedef csmFloat32 (*PhysicsValueGetter)(
    CubismVector2 translation,
    CubismPhysicsParticle* particles,
    csmInt32 particleIndex,
    csmInt32 isInverted,
    CubismVector2 parentGravity
);

typedef csmFloat32 (*PhysicsScaleGetter)(CubismVector2 translationScale, csmFloat32 angleScale);

struct CubismPhysicsInput
{
    CubismPhysicsParameter Source;
    csmInt32 SourceParameterIndex;
    csmFloat32 Weight;
    csmInt16 Type;
    csmInt16 Reflect;
    NormalizedPhysicsParameterValueGetter GetNormalizedParameterValue;
};

struct CubismPhysicsOutput
{
    CubismPhysicsParameter Destination;
    csmInt32 DestinationParameterIndex;
    csmInt32 VertexIndex;
    CubismVector2 TranslationScale;
    csmFloat32 AngleScale;
    csmFloat32 Weight;
    CubismPhysicsSource Type;
    csmInt16 Reflect;
    csmFloat32 ValueBelowMinimum;
    csmFloat32 ValueExceededMaximum;
    PhysicsValueGetter GetValue;
    PhysicsScaleGetter GetScale;
};

struct CubismPhysicsRig
{
    csmInt32 SubRigCount;
    csmVector<CubismPhysicsSubRig> Settings;
    csmVector<CubismPhysicsInput> Inputs;
    csmVector<CubismPhysicsOutput> Outputs;
    csmVector<CubismPhysicsParticle> Particles;
    CubismVector2 Gravity;
    CubismVector2 Wind;
    csmFloat32 Fps;
};

}}}
