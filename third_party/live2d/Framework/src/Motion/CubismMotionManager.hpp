

#pragma once

#include "Model/CubismModel.hpp"
#include "ACubismMotion.hpp"
#include "CubismMotionQueueManager.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

class CubismMotionManager : public CubismMotionQueueManager
{
public:
    CubismMotionManager();

    virtual ~CubismMotionManager();

    csmInt32 GetCurrentPriority() const;

    csmInt32 GetReservePriority() const;

    void SetReservePriority(csmInt32 val);

    CubismMotionQueueEntryHandle StartMotionPriority(ACubismMotion* motion, csmBool autoDelete, csmInt32 priority);

    csmBool UpdateMotion(CubismModel* model, csmFloat32 deltaTimeSeconds);

    csmBool ReserveMotion(csmInt32 priority);

private:
    csmInt32 _currentPriority;
    csmInt32 _reservePriority;
};

}}}
