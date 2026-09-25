

#pragma once

#include "Model/CubismModel.hpp"
#include "ACubismMotion.hpp"
#include "CubismMotionQueueManager.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

class CubismExpressionMotionManager : public CubismMotionQueueManager
{
public:
    struct ExpressionParameterValue
    {
        CubismIdHandle      ParameterId;
        csmFloat32          AdditiveValue;
        csmFloat32          MultiplyValue;
        csmFloat32          OverwriteValue;
    };

    CubismExpressionMotionManager();

    virtual ~CubismExpressionMotionManager();

    csmInt32 GetCurrentPriority() const;

    csmInt32 GetReservePriority() const;

    void SetReservePriority(csmInt32 priority);

    CubismMotionQueueEntryHandle StartMotionPriority(ACubismMotion* motion, csmBool autoDelete, csmInt32 priority);

    csmBool UpdateMotion(CubismModel* model, csmFloat32 deltaTimeSeconds);

    csmFloat32 GetFadeWeight(csmInt32 index);

private:

    csmVector<ExpressionParameterValue>* _expressionParameterValues;

    csmVector<csmFloat32> _fadeWeights;

    csmInt32 _currentPriority;
    csmInt32 _reservePriority;
};

}}}
