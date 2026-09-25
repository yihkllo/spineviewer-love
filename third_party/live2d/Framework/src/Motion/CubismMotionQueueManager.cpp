

#include "CubismMotionQueueManager.hpp"
#include "CubismMotionQueueEntry.hpp"
#include "CubismFramework.hpp"
#include "CubismMotion.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

const CubismMotionQueueEntryHandle InvalidMotionQueueEntryHandleValue = reinterpret_cast<CubismMotionQueueEntryHandle*>(-1);

CubismMotionQueueManager::CubismMotionQueueManager()
    : _userTimeSeconds(0.0f)
    , _eventCallback(NULL)
    , _eventCustomData(NULL)
{}

CubismMotionQueueManager::~CubismMotionQueueManager()
{
    for (csmUint32 i = 0; i < _motions.GetSize(); ++i)
    {
        if (_motions[i])
        {
            CSM_DELETE(_motions[i]);
        }
    }
}

CubismMotionQueueEntryHandle CubismMotionQueueManager::StartMotion(ACubismMotion* motion, csmBool autoDelete)
{
    if (motion == NULL)
    {
        return InvalidMotionQueueEntryHandleValue;
    }

    CubismMotionQueueEntry* motionQueueEntry = NULL;

    for (csmUint32 i = 0; i < _motions.GetSize(); ++i)
    {
        motionQueueEntry = _motions.At(i);
        if (motionQueueEntry == NULL)
        {
            continue;
        }

        motionQueueEntry->SetFadeout(motionQueueEntry->_motion->GetFadeOutTime());
    }

    motionQueueEntry = CSM_NEW CubismMotionQueueEntry();
    motionQueueEntry->_autoDelete = autoDelete;
    motionQueueEntry->_motion = motion;

    _motions.PushBack(motionQueueEntry, false);

    return motionQueueEntry->_motionQueueEntryHandle;
}

CubismMotionQueueEntryHandle CubismMotionQueueManager::StartMotion(ACubismMotion* motion, csmBool autoDelete, csmFloat32 userTimeSeconds)
{
#if _DEBUG
    CubismLogWarning("StartMotion(ACubismMotion* motion, csmBool autoDelete, csmFloat32 userTimeSeconds) is a deprecated function. Please use StartMotion(ACubismMotion* motion, csmBool autoDelete).");
#endif

    if (motion == NULL)
    {
        return InvalidMotionQueueEntryHandleValue;
    }

    CubismMotionQueueEntry* motionQueueEntry = NULL;

    for (csmUint32 i = 0; i < _motions.GetSize(); ++i)
    {
        motionQueueEntry = _motions.At(i);
        if (motionQueueEntry == NULL)
        {
            continue;
        }

        motionQueueEntry->SetFadeout(motionQueueEntry->_motion->GetFadeOutTime());
    }

    motionQueueEntry = CSM_NEW CubismMotionQueueEntry();
    motionQueueEntry->_autoDelete = autoDelete;
    motionQueueEntry->_motion = motion;

    _motions.PushBack(motionQueueEntry, false);

    return motionQueueEntry->_motionQueueEntryHandle;
}

csmBool CubismMotionQueueManager::DoUpdateMotion(CubismModel* model, csmFloat32 userTimeSeconds)
{
    csmBool updated = false;


    for (csmVector<CubismMotionQueueEntry*>::iterator ite = _motions.Begin(); ite != _motions.End();)
    {
        CubismMotionQueueEntry* motionQueueEntry = *ite;

        if (motionQueueEntry == NULL)
        {
            ite = _motions.Erase(ite);
            continue;
        }

        ACubismMotion* motion = motionQueueEntry->_motion;

        if (motion == NULL)
        {
            CSM_DELETE(motionQueueEntry);
            ite = _motions.Erase(ite);

            continue;
        }

        motion->UpdateParameters(model, motionQueueEntry, userTimeSeconds);
        updated = true;

        const csmVector<const csmString*>& firedList = motion->GetFiredEvent(
            motionQueueEntry->GetLastCheckEventTime() - motionQueueEntry->GetStartTime()
            , userTimeSeconds - motionQueueEntry->GetStartTime()
        );

        for (csmUint32 i = 0; i < firedList.GetSize(); ++i)
        {
            _eventCallback(this, *(firedList[i]), _eventCustomData);
        }

        motionQueueEntry->SetLastCheckEventTime(userTimeSeconds);

        if (motionQueueEntry->IsFinished())
        {
            CSM_DELETE(motionQueueEntry);
            ite = _motions.Erase(ite);
        }
        else
        {
            if (motionQueueEntry->IsTriggeredFadeOut())
            {
                motionQueueEntry->StartFadeout(motionQueueEntry->GetFadeOutSeconds(), userTimeSeconds);
            }

            ++ite;
        }
    }

    return updated;
}

csmVector<CubismMotionQueueEntry*>* CubismMotionQueueManager::GetCubismMotionQueueEntries()
{
    return &_motions;
}

CubismMotionQueueEntry* CubismMotionQueueManager::GetCubismMotionQueueEntry(CubismMotionQueueEntryHandle motionQueueEntryNumber)
{

    for (csmVector<CubismMotionQueueEntry*>::iterator ite = _motions.Begin(); ite != _motions.End(); ++ite)
    {
        CubismMotionQueueEntry* motionQueueEntry = *ite;

        if (motionQueueEntry == NULL)
        {
            continue;
        }

        if (motionQueueEntry->_motionQueueEntryHandle == motionQueueEntryNumber)
        {
            return motionQueueEntry;
        }
    }

    return NULL;
}

csmBool CubismMotionQueueManager::IsFinished()
{

    for (csmVector<CubismMotionQueueEntry*>::iterator ite = _motions.Begin(); ite != _motions.End();)
    {
        CubismMotionQueueEntry* motionQueueEntry = *ite;

        if (motionQueueEntry == NULL)
        {
            ite = _motions.Erase(ite);
            continue;
        }

        ACubismMotion* motion = motionQueueEntry->_motion;

        if (motion == NULL)
        {
            CSM_DELETE(motionQueueEntry);
            ite = _motions.Erase(ite);
            continue;
        }

        if (!motionQueueEntry->IsFinished())
        {
            return false;
        }
        else
        {
            ++ite;
        }
    }

    return true;
}

csmBool CubismMotionQueueManager::IsFinished(CubismMotionQueueEntryHandle motionQueueEntryNumber)
{

    for (csmVector<CubismMotionQueueEntry*>::iterator ite = _motions.Begin(); ite != _motions.End(); ite++)
    {
        CubismMotionQueueEntry* motionQueueEntry = *ite;

        if (motionQueueEntry == NULL)
        {
            continue;
        }

        if (motionQueueEntry->_motionQueueEntryHandle == motionQueueEntryNumber && !motionQueueEntry->IsFinished())
        {
            return false;
        }
    }

    return true;
}

void CubismMotionQueueManager::StopAllMotions()
{

    for (csmVector<CubismMotionQueueEntry*>::iterator ite = _motions.Begin(); ite != _motions.End();)
    {
        CubismMotionQueueEntry* motionQueueEntry = *ite;

        if (motionQueueEntry == NULL)
        {
            ite = _motions.Erase(ite);

            continue;
        }

        CSM_DELETE(motionQueueEntry);
        ite = _motions.Erase(ite);
    }
}

void CubismMotionQueueManager::SetEventCallback(CubismMotionEventFunction callback, void* customData)
{
    _eventCallback   = callback;
    _eventCustomData = customData;
}

}}}
