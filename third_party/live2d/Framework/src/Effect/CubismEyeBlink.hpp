

#pragma once

#include "Model/CubismModel.hpp"
#include "Type/csmVector.hpp"
#include "Id/CubismId.hpp"
#include "ICubismModelSetting.hpp"


namespace Live2D { namespace Cubism { namespace Framework {

class CubismEyeBlink
{
public:
    enum EyeState
    {
        EyeState_First = 0,
        EyeState_Interval,
        EyeState_Closing,
        EyeState_Closed,
        EyeState_Opening
    };

    static CubismEyeBlink* Create(ICubismModelSetting* modelSetting = NULL);

    static void Delete(CubismEyeBlink* eyeBlink);

    void            SetBlinkingInterval(csmFloat32 blinkingInterval);

    void            SetBlinkingSettings(csmFloat32 closing, csmFloat32 closed, csmFloat32 opening);

    void            SetParameterIds(const csmVector<CubismIdHandle>& parameterIds);

    const csmVector<CubismIdHandle>&     GetParameterIds() const;

    void            UpdateParameters(CubismModel* model, csmFloat32 deltaTimeSeconds);

private:

    CubismEyeBlink(ICubismModelSetting* modelSetting);

    virtual ~CubismEyeBlink();

    csmFloat32        DetermineNextBlinkingTiming() const;

    csmInt32                    _blinkingState;
    csmVector<CubismIdHandle>   _parameterIds;
    csmFloat32                  _nextBlinkingTime;
    csmFloat32                  _stateStartTimeSeconds;
    csmFloat32                  _blinkingIntervalSeconds;
    csmFloat32                  _closingSeconds;
    csmFloat32                  _closedSeconds;
    csmFloat32                  _openingSeconds;
    csmFloat32                  _userTimeSeconds;

};

}}}
