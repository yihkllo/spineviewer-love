

#pragma once

#include "CubismFramework.hpp"
#include "Type/csmMap.hpp"
#include "Id/CubismId.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

class ICubismModelSetting
{
public:
    virtual ~ICubismModelSetting() {}

    virtual const csmChar* GetModelFileName() = 0;

    virtual csmInt32 GetTextureCount() = 0;

    virtual const csmChar* GetTextureDirectory() = 0;

    virtual const csmChar* GetTextureFileName(csmInt32 index) = 0;

    virtual csmInt32 GetHitAreasCount() = 0;

    virtual CubismIdHandle GetHitAreaId(csmInt32 index) = 0;

    virtual const csmChar* GetHitAreaName(csmInt32 index) = 0;

    virtual const csmChar* GetPhysicsFileName() = 0;

    virtual const csmChar* GetPoseFileName() = 0;

    virtual const csmChar* GetDisplayInfoFileName() = 0;

    virtual csmInt32 GetExpressionCount() = 0;

    virtual const csmChar* GetExpressionName(csmInt32 index) = 0;

    virtual const csmChar* GetExpressionFileName(csmInt32 index) = 0;

    virtual csmInt32 GetMotionGroupCount() = 0;

    virtual const csmChar* GetMotionGroupName(csmInt32 index) = 0;

    virtual csmInt32 GetMotionCount(const csmChar* groupName) = 0;

    virtual const csmChar* GetMotionFileName(const csmChar* groupName, csmInt32 index) = 0;

    virtual const csmChar* GetMotionSoundFileName(const csmChar* groupName, csmInt32 index) = 0;

    virtual csmFloat32 GetMotionFadeInTimeValue(const csmChar* groupName, csmInt32 index) = 0;

    virtual csmFloat32 GetMotionFadeOutTimeValue(const csmChar* groupName, csmInt32 index) = 0;

    virtual const csmChar* GetUserDataFile() = 0;

    virtual csmBool GetLayoutMap(csmMap<csmString, csmFloat32>& outLayoutMap) = 0;

    virtual csmInt32 GetEyeBlinkParameterCount() = 0;

    virtual CubismIdHandle GetEyeBlinkParameterId(csmInt32 index) = 0;

    virtual csmInt32 GetLipSyncParameterCount() = 0;

    virtual CubismIdHandle GetLipSyncParameterId(csmInt32 index) = 0;
};
}}}
