

#include "CubismExpressionMotion.hpp"
#include "CubismMotionQueueEntry.hpp"
#include "Id/CubismIdManager.hpp"
#include "Math/CubismMath.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

const csmFloat32 CubismExpressionMotion::DefaultAdditiveValue = 0.0f;
const csmFloat32 CubismExpressionMotion::DefaultMultiplyValue = 1.0f;

namespace {
const csmChar* ExpressionKeyFadeIn = "FadeInTime";
const csmChar* ExpressionKeyFadeOut = "FadeOutTime";
const csmChar* ExpressionKeyParameters = "Parameters";
const csmChar* ExpressionKeyId = "Id";
const csmChar* ExpressionKeyValue = "Value";
const csmChar* ExpressionKeyBlend = "Blend";
const csmChar* BlendValueAdd = "Add";
const csmChar* BlendValueMultiply = "Multiply";
const csmChar* BlendValueOverwrite = "Overwrite";
const csmFloat32 DefaultFadeTime = 1.0f;
}


CubismExpressionMotion::CubismExpressionMotion()
{ }

CubismExpressionMotion::~CubismExpressionMotion()
{ }

CubismExpressionMotion* CubismExpressionMotion::Create(const csmByte* buffer, csmSizeInt size)
{
    CubismExpressionMotion* expression = CSM_NEW CubismExpressionMotion();
    expression->Parse(buffer, size);
    return expression;
}

void CubismExpressionMotion::DoUpdateParameters(CubismModel* model, csmFloat32 userTimeSeconds, csmFloat32 weight, CubismMotionQueueEntry* motionQueueEntry)
{
    for (csmUint32 i = 0; i < _parameters.GetSize(); ++i)
    {
        ExpressionParameter& parameter = _parameters[i];

        switch (parameter.BlendType)
        {
        case Additive: {
            model->AddParameterValue(parameter.ParameterId, parameter.Value, weight);
            break;
        }
        case Multiply: {
            model->MultiplyParameterValue(parameter.ParameterId, parameter.Value, weight);
            break;
        }
        case Overwrite: {
            model->SetParameterValue(parameter.ParameterId, parameter.Value, weight);
            break;
        }
        default:
            break;
        }
    }
}

void CubismExpressionMotion::CalculateExpressionParameters(CubismModel* model, csmFloat32 userTimeSeconds, CubismMotionQueueEntry* motionQueueEntry,
    csmVector<CubismExpressionMotionManager::ExpressionParameterValue>* expressionParameterValues, csmInt32 expressionIndex, csmFloat32 fadeWeight)
{
    if (motionQueueEntry == NULL || expressionParameterValues == NULL)
    {
        return;
    }

    if (!motionQueueEntry->IsAvailable())
    {
        return;
    }

    _fadeWeight = UpdateFadeWeight(motionQueueEntry, userTimeSeconds);

    for (csmInt32 i = 0; i < expressionParameterValues->GetSize(); ++i)
    {
        CubismExpressionMotionManager::ExpressionParameterValue expressionParameterValue = expressionParameterValues->At(i);

        if (expressionParameterValue.ParameterId == NULL)
        {
            continue;
        }

        const csmFloat32 currentParameterValue = expressionParameterValue.OverwriteValue =
            model->GetParameterValue(expressionParameterValue.ParameterId);

        csmVector<ExpressionParameter> expressionParameters = GetExpressionParameters();
        csmInt32 parameterIndex = -1;
        for (csmInt32 j = 0; j < expressionParameters.GetSize(); ++j)
        {
            if (expressionParameterValue.ParameterId != expressionParameters[j].ParameterId)
            {
                continue;
            }

            parameterIndex = j;

            break;
        }

        if (parameterIndex < 0)
        {
            if (expressionIndex == 0)
            {
                expressionParameterValues->At(i).AdditiveValue = DefaultAdditiveValue;

                expressionParameterValues->At(i).MultiplyValue = DefaultMultiplyValue;

                expressionParameterValues->At(i).OverwriteValue = currentParameterValue;
            }
            else
            {
                expressionParameterValues->At(i).AdditiveValue =
                    CalculateValue(expressionParameterValue.AdditiveValue, DefaultAdditiveValue, fadeWeight);

                expressionParameterValues->At(i).MultiplyValue =
                    CalculateValue(expressionParameterValue.MultiplyValue, DefaultMultiplyValue, fadeWeight);

                expressionParameterValues->At(i).OverwriteValue =
                    CalculateValue(expressionParameterValue.OverwriteValue, currentParameterValue, fadeWeight);
            }
            continue;
        }

        csmFloat32 value = expressionParameters.At(parameterIndex).Value;
        csmFloat32 newAdditiveValue, newMultiplyValue, newSetValue;
        switch (expressionParameters.At(parameterIndex).BlendType) {
        case Additive:
            newAdditiveValue = value;
            newMultiplyValue = DefaultMultiplyValue;
            newSetValue = currentParameterValue;
            break;
        case Multiply:
            newAdditiveValue = DefaultAdditiveValue;
            newMultiplyValue = value;
            newSetValue = currentParameterValue;
            break;
        case Overwrite:
            newAdditiveValue = DefaultAdditiveValue;
            newMultiplyValue = DefaultMultiplyValue;
            newSetValue = value;
            break;
        default:
            return;
        }

        if (expressionIndex == 0) {
            expressionParameterValues->At(i).AdditiveValue = newAdditiveValue;
            expressionParameterValues->At(i).MultiplyValue = newMultiplyValue;
            expressionParameterValues->At(i).OverwriteValue = newSetValue;
        }
        else {
            expressionParameterValues->At(i).AdditiveValue = (expressionParameterValue.AdditiveValue * (1.0f - fadeWeight)) + newAdditiveValue * fadeWeight;
            expressionParameterValues->At(i).MultiplyValue = (expressionParameterValue.MultiplyValue * (1.0f - fadeWeight)) + newMultiplyValue * fadeWeight;
            expressionParameterValues->At(i).OverwriteValue = (expressionParameterValue.OverwriteValue * (1.0f - fadeWeight)) + newSetValue * fadeWeight;
        }
    }
}

csmVector<CubismExpressionMotion::ExpressionParameter> CubismExpressionMotion::GetExpressionParameters()
{
    return _parameters;
}

csmFloat32 CubismExpressionMotion::GetFadeWeight()
{
#if _DEBUG
    CubismLogWarning("GetFadeWeight() is a deprecated function. Please use CubismExpressionMotionManager.GetFadeWeight(int index).");
#endif

    return _fadeWeight;
}

void CubismExpressionMotion::Parse(const csmByte* buffer, csmSizeInt size)
{
    Utils::CubismJson* json = Utils::CubismJson::Create(buffer, size);
    if (!json)
    {
        return;
    }

    Utils::Value& root = json->GetRoot();

    SetFadeInTime(root[ExpressionKeyFadeIn].ToFloat(DefaultFadeTime));
    SetFadeOutTime(root[ExpressionKeyFadeOut].ToFloat(DefaultFadeTime));

    const csmInt32 parameterCount = root[ExpressionKeyParameters].GetSize();
    _parameters.PrepareCapacity(parameterCount);

    for (csmInt32 i = 0; i < parameterCount; ++i)
    {
        Utils::Value& param = root[ExpressionKeyParameters][i];
        const CubismIdHandle parameterId = CubismFramework::GetIdManager()->GetId(param[ExpressionKeyId].GetRawString());
        const csmFloat32 value = static_cast<csmFloat32>(param[ExpressionKeyValue].ToFloat());

        ExpressionBlendType blendType;

        if (param[ExpressionKeyBlend].IsNull() || param[ExpressionKeyBlend].GetString() == BlendValueAdd)
        {
            blendType = Additive;
        }
        else if (param[ExpressionKeyBlend].GetString() == BlendValueMultiply)
        {
            blendType = Multiply;
        }
        else if (param[ExpressionKeyBlend].GetString() == BlendValueOverwrite)
        {
            blendType = Overwrite;
        }
        else
        {
            blendType = Additive;
        }

        ExpressionParameter item;

        item.ParameterId = parameterId;
        item.BlendType = blendType;
        item.Value = value;

        _parameters.PushBack(item);
    }

    Utils::CubismJson::Delete(json);
}

csmFloat32 CubismExpressionMotion::CalculateValue(csmFloat32 source, csmFloat32 destination, csmFloat32 fadeWeight)
{
    return (source * (1.0f - fadeWeight)) + (destination * fadeWeight);
}


}}}
