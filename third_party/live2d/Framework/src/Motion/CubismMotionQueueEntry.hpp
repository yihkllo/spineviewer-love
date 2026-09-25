

#pragma once

#include "ACubismMotion.hpp"
#include "Type/csmVector.hpp"
#include "Model/CubismUserModel.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

class CubismMotion;

class CubismMotionQueueEntry
{
    friend class CubismMotionQueueManager;
    friend class ACubismMotion;
    friend class CubismMotion;

public:
    CubismMotionQueueEntry();

    virtual ~CubismMotionQueueEntry();

    void        SetFadeout(csmFloat32 fadeOutSeconds);

    void        StartFadeout(csmFloat32 fadeOutSeconds, csmFloat32 userTimeSeconds);

    csmBool     IsFinished() const;

    csmBool     IsStarted() const;

    csmFloat32    GetStartTime() const;

    csmFloat32    GetFadeInStartTime() const;

    csmFloat32    GetEndTime() const;

    void        SetStartTime(csmFloat32 startTime);

    void        SetFadeInStartTime(csmFloat32 startTime);

    void        SetEndTime(csmFloat32 endTime);

    void        IsFinished(csmBool f);

    void        IsStarted(csmBool f);

    csmBool     IsAvailable() const;

    void        IsAvailable(csmBool v);

    void        SetState(csmFloat32 timeSeconds, csmFloat32 weight);

    csmFloat32  GetStateTime() const;

    csmFloat32  GetStateWeight() const;

    csmFloat32  GetLastCheckEventTime() const;

    void        SetLastCheckEventTime(csmFloat32 checkTime);

    csmBool     IsTriggeredFadeOut();

    csmFloat32     GetFadeOutSeconds();

    ACubismMotion* GetCubismMotion();

private:
    csmBool         _autoDelete;
    ACubismMotion*  _motion;

    csmBool         _available;
    csmBool         _finished;
    csmBool         _started;
    csmFloat32      _startTimeSeconds;
    csmFloat32      _fadeInStartTimeSeconds;
    csmFloat32      _endTimeSeconds;
    csmFloat32      _stateTimeSeconds;
    csmFloat32      _stateWeight;
    csmFloat32      _lastEventCheckSeconds;
    csmFloat32      _fadeOutSeconds;
    csmBool         _IsTriggeredFadeOut;

    CubismMotionQueueEntryHandle  _motionQueueEntryHandle;
};

}}}
