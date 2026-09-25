

#pragma once

#include "ICubismModelSetting.hpp"
#include "CubismJsonHolder.hpp"
#include "Utils/CubismJson.hpp"
#include "Id/CubismId.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

class CubismModelSettingJson : public ICubismModelSetting, public CubismJsonHolder
{
public:

    CubismModelSettingJson(const csmByte* buffer, csmSizeInt size);

    virtual ~CubismModelSettingJson();

    Utils::CubismJson* GetJsonPointer() const;

    const csmChar* GetModelFileName();

    csmInt32 GetTextureCount();

    const csmChar* GetTextureDirectory();

    const csmChar* GetTextureFileName(csmInt32 index);

    csmInt32 GetHitAreasCount();

    CubismIdHandle GetHitAreaId(csmInt32 index);

    const csmChar* GetHitAreaName(csmInt32 index);

    const csmChar* GetPhysicsFileName();

    const csmChar* GetPoseFileName();

    const csmChar* GetDisplayInfoFileName();

    csmInt32 GetExpressionCount();

    const csmChar* GetExpressionName(csmInt32 index);

    const csmChar* GetExpressionFileName(csmInt32 index);

    csmInt32 GetMotionGroupCount();

    const csmChar* GetMotionGroupName(csmInt32 index);

    csmInt32 GetMotionCount(const csmChar* groupName);

    const csmChar* GetMotionFileName(const csmChar* groupName, csmInt32 index);

    const csmChar* GetMotionSoundFileName(const csmChar* groupName, csmInt32 index);

    csmFloat32 GetMotionFadeInTimeValue(const csmChar* groupName, csmInt32 index);

    csmFloat32 GetMotionFadeOutTimeValue(const csmChar* groupName, csmInt32 index);

    const csmChar* GetUserDataFile();

    csmBool GetLayoutMap(csmMap<csmString, csmFloat32>& outLayoutMap);

    csmInt32 GetEyeBlinkParameterCount();

    CubismIdHandle GetEyeBlinkParameterId(csmInt32 index);

    csmInt32 GetLipSyncParameterCount();

    CubismIdHandle GetLipSyncParameterId(csmInt32 index);

private:

    enum FrequentNode
    {
        FrequentNode_Groups,
        FrequentNode_Moc,
        FrequentNode_Motions,
        FrequentNode_DisplayInfo,
        FrequentNode_Expressions,
        FrequentNode_Textures,
        FrequentNode_Physics,
        FrequentNode_Pose,
        FrequentNode_HitAreas,
    };

    csmBool IsExistModelFile() const;

    csmBool IsExistTextureFiles() const;

    csmBool IsExistHitAreas() const;

    csmBool IsExistPhysicsFile() const;

    csmBool IsExistPoseFile() const;

    csmBool IsExistDisplayInfoFile() const;

    csmBool IsExistExpressionFile() const;

    csmBool IsExistMotionGroups() const;

    csmBool IsExistMotionGroupName(const csmChar* groupName) const;

    csmBool IsExistMotionSoundFile(const csmChar* groupName, csmInt32 index) const;

    csmBool IsExistMotionFadeIn(const csmChar* groupName, csmInt32 index) const;

    csmBool IsExistMotionFadeOut(const csmChar* groupName, csmInt32 index) const;

    csmBool IsExistUserDataFile() const;

    csmBool IsExistEyeBlinkParameters() const;

    csmBool IsExistLipSyncParameters() const;

    csmVector<Utils::Value*>    _jsonValue;
};
}}}
