

#include "CubismExpressionMotionManager.hpp"
#include "CubismExpressionMotion.hpp"
#include "CubismMotionQueueEntry.hpp"
#include "CubismFramework.hpp"
#include "Math/CubismMath.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

CubismExpressionMotionManager::CubismExpressionMotionManager()
    : _currentPriority(0)
    , _reservePriority(0)
    , _expressionParameterValues(CSM_NEW csmVector<ExpressionParameterValue>())
{ }

CubismExpressionMotionManager::~CubismExpressionMotionManager()
{
    if(_expressionParameterValues)
    {
        CSM_DELETE(_expressionParameterValues);

        _expressionParameterValues = NULL;
    }

    _fadeWeights.Clear();
}

csmInt32 CubismExpressionMotionManager::GetCurrentPriority() const
{
    return _currentPriority;
}

csmInt32 CubismExpressionMotionManager::GetReservePriority() const
{
    return _reservePriority;
}

void CubismExpressionMotionManager::SetReservePriority(csmInt32 priority)
{
    _reservePriority = priority;
}

CubismMotionQueueEntryHandle CubismExpressionMotionManager::StartMotionPriority(ACubismMotion* motion, csmBool autoDelete, csmInt32 priority)
{
    if (priority == _reservePriority)
    {
        _reservePriority = 0;
    }
    _currentPriority = priority;

    _fadeWeights.PushBack(0.0f);

    return CubismMotionQueueManager::StartMotion(motion, autoDelete);
}

csmBool CubismExpressionMotionManager::UpdateMotion(CubismModel* model, csmFloat32 deltaTimeSeconds)
{
    _userTimeSeconds += deltaTimeSeconds;
    csmBool updated = false;
    csmVector<CubismMotionQueueEntry*>* motions = GetCubismMotionQueueEntries();

    csmFloat32 expressionWeight = 0.0f;
    csmInt32 expressionIndex = 0;

    for (csmVector<CubismMotionQueueEntry*>::iterator ite = motions->Begin(); ite != motions->End();)
    {
        CubismMotionQueueEntry* motionQueueEntry = *ite;

        if (motionQueueEntry == NULL)
        {
            ite = motions->Erase(ite);
            continue;
        }

        CubismExpressionMotion* expressionMotion = (CubismExpressionMotion*)motionQueueEntry->GetCubismMotion();

        if (expressionMotion == NULL)
        {
            CSM_DELETE(motionQueueEntry);
            ite = motions->Erase(ite);
            continue;
        }

        csmVector<CubismExpressionMotion::ExpressionParameter> expressionParameters = expressionMotion->GetExpressionParameters();
        if (motionQueueEntry->IsAvailable())
        {
            for (csmInt32 i = 0; i < expressionParameters.GetSize(); ++i)
            {
                if (expressionParameters[i].ParameterId == NULL)
                {
                    continue;
                }

                csmInt32 index = -1;
                for (csmInt32 j = 0; j < _expressionParameterValues->GetSize(); ++j)
                {
                    if (_expressionParameterValues->At(j).ParameterId != expressionParameters[i].ParameterId)
                    {
                        continue;
                    }

                    index = j;
                    break;
                }

                if (index >= 0)
                {
                    continue;
                }

                ExpressionParameterValue item;
                item.ParameterId = expressionParameters[i].ParameterId;
                item.AdditiveValue = CubismExpressionMotion::DefaultAdditiveValue;
                item.MultiplyValue = CubismExpressionMotion::DefaultMultiplyValue;
                item.OverwriteValue = model->GetParameterValue(item.ParameterId);
                _expressionParameterValues->PushBack(item);
            }
        }

        expressionMotion->SetupMotionQueueEntry(motionQueueEntry, _userTimeSeconds);
        _fadeWeights[expressionIndex] = expressionMotion->UpdateFadeWeight(motionQueueEntry, _userTimeSeconds);
        expressionMotion->CalculateExpressionParameters(model, _userTimeSeconds, motionQueueEntry,
            _expressionParameterValues, expressionIndex, _fadeWeights[expressionIndex]);

        expressionWeight += expressionMotion->GetFadeInTime() == 0.0f
            ? 1.0f
            : CubismMath::GetEasingSine((_userTimeSeconds - motionQueueEntry->GetFadeInStartTime()) / expressionMotion->GetFadeInTime());

        updated = true;

        if (motionQueueEntry->IsTriggeredFadeOut())
        {
            motionQueueEntry->StartFadeout(motionQueueEntry->GetFadeOutSeconds(), _userTimeSeconds);
        }

        ++ite;
        ++expressionIndex;
    }

    if (motions->GetSize() > 1)
    {
        CubismExpressionMotion* expressionMotion =
            (CubismExpressionMotion*)(motions->At(motions->GetSize() - 1))->GetCubismMotion();

        csmFloat32 latestFadeWeight = _fadeWeights[_fadeWeights.GetSize() - 1];
        if (latestFadeWeight >= 1.0f)
        {
            for (csmInt32 i = motions->GetSize()-2; i >= 0; i--)
            {
                CubismMotionQueueEntry* motionQueueEntry = motions->At(i);
                CSM_DELETE(motionQueueEntry);
                motions->Remove(i);
                _fadeWeights.Remove(i);
            }
        }
    }

    if (expressionWeight > 1.0f)
    {
        expressionWeight = 1.0f;
    }

    for (csmInt32 i = 0; i < _expressionParameterValues->GetSize(); ++i)
    {
        model->SetParameterValue(_expressionParameterValues->At(i).ParameterId,
            (_expressionParameterValues->At(i).OverwriteValue + _expressionParameterValues->At(i).AdditiveValue) * _expressionParameterValues->At(i).MultiplyValue,
            expressionWeight);

        _expressionParameterValues->At(i).AdditiveValue = CubismExpressionMotion::DefaultAdditiveValue;
        _expressionParameterValues->At(i).MultiplyValue = CubismExpressionMotion::DefaultMultiplyValue;
    }

    return updated;
}

csmFloat32 CubismExpressionMotionManager::GetFadeWeight(csmInt32 index)
{
    return _fadeWeights[index];
}

}}}
