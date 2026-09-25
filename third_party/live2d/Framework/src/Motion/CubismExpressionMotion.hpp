

#pragma once

#include "ACubismMotion.hpp"
#include "Utils/CubismJson.hpp"
#include "Model/CubismModel.hpp"
#include "CubismExpressionMotionManager.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

class CubismExpressionMotion : public ACubismMotion
{
public:
    enum ExpressionBlendType
    {
        Additive = 0,
        Multiply = 1,
        Overwrite = 2
    };

    struct ExpressionParameter
    {
        CubismIdHandle      ParameterId;
        ExpressionBlendType BlendType;
        csmFloat32          Value;
    };

    static CubismExpressionMotion* Create(const csmByte* buf, csmSizeInt size);

    virtual void DoUpdateParameters(CubismModel* model, csmFloat32 userTimeSeconds, csmFloat32 weight, CubismMotionQueueEntry* motionQueueEntry);

    void CalculateExpressionParameters(CubismModel* model, csmFloat32 userTimeSeconds, CubismMotionQueueEntry* motionQueueEntry,
        csmVector<CubismExpressionMotionManager::ExpressionParameterValue>* expressionParameterValues, csmInt32 expressionIndex, csmFloat32 fadeWeight);

    csmVector<ExpressionParameter> GetExpressionParameters();

    csmFloat32 GetFadeWeight();

    static const csmFloat32 DefaultAdditiveValue;
    static const csmFloat32 DefaultMultiplyValue;

protected:
    CubismExpressionMotion();

    virtual ~CubismExpressionMotion();

    void Parse(const csmByte* exp3Json, csmSizeInt size);

    csmVector<ExpressionParameter> _parameters;

private:

    csmFloat32 CalculateValue(csmFloat32 source, csmFloat32 destination, csmFloat32 fadeWeight);


    csmFloat32 _fadeWeight;
};

}}}
