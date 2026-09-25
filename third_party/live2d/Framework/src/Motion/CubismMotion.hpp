

#pragma once

#include "ACubismMotion.hpp"
#include "Type/CubismBasicType.hpp"
#include "Type/csmVector.hpp"
#include "Id/CubismId.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

class CubismMotionQueueEntry;
struct CubismMotionData;

class CubismMotion : public ACubismMotion
{
public:
    static CubismMotion* Create(const csmByte* buffer, csmSizeInt size, FinishedMotionCallback onFinishedMotionHandler = NULL);

    virtual void        DoUpdateParameters(CubismModel* model, csmFloat32 userTimeSeconds, csmFloat32 fadeWeight, CubismMotionQueueEntry* motionQueueEntry);

    void                IsLoop(csmBool loop);

    csmBool             IsLoop() const;

    void                IsLoopFadeIn(csmBool loopFadeIn);

    csmBool             IsLoopFadeIn() const;

    virtual csmFloat32  GetDuration();

    virtual csmFloat32  GetLoopDuration();

    void        SetParameterFadeInTime(CubismIdHandle parameterId, csmFloat32 value);

    void        SetParameterFadeOutTime(CubismIdHandle parameterId, csmFloat32 value);

    csmFloat32    GetParameterFadeInTime(CubismIdHandle parameterId) const;

    csmFloat32    GetParameterFadeOutTime(CubismIdHandle parameterId) const;

    void SetEffectIds(const csmVector<CubismIdHandle>& eyeBlinkParameterIds, const csmVector<CubismIdHandle>& lipSyncParameterIds);

    virtual const csmVector<const csmString*>& GetFiredEvent(csmFloat32 beforeCheckTimeSeconds, csmFloat32 motionTimeSeconds);

    csmBool IsExistModelOpacity() const;

    csmInt32 GetModelOpacityIndex() const;

    CubismIdHandle GetModelOpacityId(csmInt32 index);

protected:
    csmFloat32 GetModelOpacityValue() const;

private:
    CubismMotion();

    virtual ~CubismMotion();

    CubismMotion(const CubismMotion&);
    CubismMotion& operator=(const CubismMotion&);

    void Parse(const csmByte* motionJson, const csmSizeInt size);

    csmFloat32      _sourceFrameRate;
    csmFloat32      _loopDurationSeconds;
    csmBool         _isLoop;
    csmBool         _isLoopFadeIn;
    csmFloat32      _lastWeight;

    CubismMotionData*    _motionData;

    csmVector<CubismIdHandle>  _eyeBlinkParameterIds;
    csmVector<CubismIdHandle>  _lipSyncParameterIds;

    CubismIdHandle _modelCurveIdEyeBlink;
    CubismIdHandle _modelCurveIdLipSync;
    CubismIdHandle _modelCurveIdOpacity;

    csmFloat32 _modelOpacity;
};

}}}

