

#pragma once

#include "CubismJsonHolder.hpp"
#include "Utils/CubismJson.hpp"
#include "Math/CubismVector2.hpp"
#include "Id/CubismId.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

class CubismPhysicsJson : public CubismJsonHolder
{
public:
    CubismPhysicsJson(const csmByte* buffer, csmSizeInt size);

    virtual ~CubismPhysicsJson();

    CubismVector2 GetGravity() const;

    CubismVector2 GetWind() const;

    csmFloat32 GetFps() const;

    csmInt32 GetSubRigCount() const;

    csmInt32 GetTotalInputCount() const;

    csmInt32 GetTotalOutputCount() const;

    csmInt32 GetVertexCount() const;

    csmFloat32 GetNormalizationPositionMinimumValue(csmInt32 physicsSettingIndex) const;

    csmFloat32 GetNormalizationPositionMaximumValue(csmInt32 physicsSettingIndex) const;

    csmFloat32 GetNormalizationPositionDefaultValue(csmInt32 physicsSettingIndex) const;

    csmFloat32 GetNormalizationAngleMinimumValue(csmInt32 physicsSettingIndex) const;

    csmFloat32 GetNormalizationAngleMaximumValue(csmInt32 physicsSettingIndex) const;

    csmFloat32 GetNormalizationAngleDefaultValue(csmInt32 physicsSettingIndex) const;

    csmInt32 GetInputCount(csmInt32 physicsSettingIndex) const;

    csmFloat32 GetInputWeight(csmInt32 physicsSettingIndex, csmInt32 inputIndex) const;

    csmBool GetInputReflect(csmInt32 physicsSettingIndex, csmInt32 inputIndex) const;

    const csmChar* GetInputType(csmInt32 physicsSettingIndex, csmInt32 inputIndex) const;

    CubismIdHandle GetInputSourceId(csmInt32 physicsSettingIndex, csmInt32 inputIndex) const;

    csmInt32 GetOutputCount(csmInt32 physicsSettingIndex) const;

    csmInt32 GetOutputVertexIndex(csmInt32 physicsSettingIndex, csmInt32 outputIndex) const;

    csmFloat32 GetOutputAngleScale(csmInt32 physicsSettingIndex, csmInt32 outputIndex) const;

    csmFloat32 GetOutputWeight(csmInt32 physicsSettingIndex, csmInt32 outputIndex) const;

    CubismIdHandle GetOutputsDestinationId(csmInt32 physicsSettingIndex, csmInt32 outputIndex) const;

    const csmChar* GetOutputType(csmInt32 physicsSettingIndex, csmInt32 outputIndex) const;

    csmBool GetOutputReflect(csmInt32 physicsSettingIndex, csmInt32 outputIndex) const;

    csmInt32 GetParticleCount(csmInt32 physicsSettingIndex) const;

    csmFloat32 GetParticleMobility(csmInt32 physicsSettingIndex, csmInt32 vertexIndex) const;

    csmFloat32 GetParticleDelay(csmInt32 physicsSettingIndex, csmInt32 vertexIndex) const;

    csmFloat32 GetParticleAcceleration(csmInt32 physicsSettingIndex, csmInt32 vertexIndex) const;

    csmFloat32 GetParticleRadius(csmInt32 physicsSettingIndex, csmInt32 vertexIndex) const;

    CubismVector2 GetParticlePosition(csmInt32 physicsSettingIndex, csmInt32 vertexIndex) const;
};

}}}
