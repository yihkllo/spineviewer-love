

#pragma once

#include "CubismFramework.hpp"
#include "Id/CubismId.hpp"
#include "Type/csmVector.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

class CubismMotionQueueManager;
class CubismMotionQueueEntry;
class CubismModel;

class ACubismMotion
{
public:
    typedef void (*FinishedMotionCallback)(ACubismMotion* self);
    static void Delete(ACubismMotion* motion);

    ACubismMotion();

    void UpdateParameters(CubismModel* model, CubismMotionQueueEntry* motionQueueEntry, csmFloat32 userTimeSeconds);

    void SetupMotionQueueEntry(CubismMotionQueueEntry* motionQueueEntry, csmFloat32 userTimeSeconds);

    void SetFadeInTime(csmFloat32 fadeInSeconds);

    void SetFadeOutTime(csmFloat32 fadeOutSeconds);

    csmFloat32 GetFadeOutTime() const;

    csmFloat32 GetFadeInTime() const;

    void SetWeight(csmFloat32 weight);

    csmFloat32 GetWeight() const;

    virtual csmFloat32 GetDuration();

    virtual csmFloat32 GetLoopDuration();


    void SetOffsetTime(csmFloat32 offsetSeconds);

    virtual const csmVector<const csmString*>& GetFiredEvent(csmFloat32 beforeCheckTimeSeconds,
                                                                   csmFloat32 motionTimeSeconds);


    void SetFinishedMotionHandler(FinishedMotionCallback onFinishedMotionHandler);

    FinishedMotionCallback GetFinishedMotionHandler();

    void SetFinishedMotionCustomData(void* onFinishedMotionCustomData);

    void* GetFinishedMotionCustomData();

    void SetFinishedMotionHandlerAndMotionCustomData(FinishedMotionCallback onFinishedMotionHandler, void* onFinishedMotionCustomData);

    virtual csmBool IsExistModelOpacity() const;

    virtual csmInt32 GetModelOpacityIndex() const;

    virtual CubismIdHandle GetModelOpacityId(csmInt32 index);

    csmFloat32 UpdateFadeWeight(CubismMotionQueueEntry* motionQueueEntry, csmFloat32 userTimeSeconds);

private:
    ACubismMotion(const ACubismMotion&);
    ACubismMotion& operator=(const ACubismMotion&);

protected:
    virtual ~ACubismMotion();

    virtual csmFloat32 GetModelOpacityValue() const;

    virtual void DoUpdateParameters(CubismModel* model, csmFloat32 userTimeSeconds, csmFloat32 weight, CubismMotionQueueEntry* motionQueueEntry) = 0;

    csmFloat32    _fadeInSeconds;
    csmFloat32    _fadeOutSeconds;
    csmFloat32    _weight;
    csmFloat32    _offsetSeconds;

    csmVector<const csmString*>    _firedEventValues;

    FinishedMotionCallback _onFinishedMotion;
    void* _onFinishedMotionCustomData;
};

}}}
