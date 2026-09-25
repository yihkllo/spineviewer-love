

#pragma once

#include "ACubismMotion.hpp"
#include "Model/CubismModel.hpp"
#include "Type/csmVector.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

class CubismMotionQueueEntry;
class CubismMotionQueueManager;

typedef void(*CubismMotionEventFunction)(const CubismMotionQueueManager* caller, const csmString& eventValue, void* customData);

typedef void* CubismMotionQueueEntryHandle;

extern const CubismMotionQueueEntryHandle InvalidMotionQueueEntryHandleValue;

class CubismMotionQueueManager
{
public:
    CubismMotionQueueManager();

    virtual ~CubismMotionQueueManager();

    CubismMotionQueueEntryHandle    StartMotion(ACubismMotion* motion, csmBool autoDelete);

    CubismMotionQueueEntryHandle    StartMotion(ACubismMotion* motion, csmBool autoDelete, csmFloat32 userTimeSeconds);

    csmBool     IsFinished();

    csmBool     IsFinished(CubismMotionQueueEntryHandle motionQueueEntryNumber);

    void        StopAllMotions();

    CubismMotionQueueEntry* GetCubismMotionQueueEntry(CubismMotionQueueEntryHandle motionQueueEntryNumber);

    csmVector<CubismMotionQueueEntry*>* GetCubismMotionQueueEntries();

    void SetEventCallback(CubismMotionEventFunction callback, void* customData = NULL);

protected:
    virtual csmBool     DoUpdateMotion(CubismModel* model, csmFloat32 userTimeSeconds);


    csmFloat32 _userTimeSeconds;

private:
    csmVector<CubismMotionQueueEntry*>      _motions;

    CubismMotionEventFunction         _eventCallback;
    void*                             _eventCustomData;
};

}}}
