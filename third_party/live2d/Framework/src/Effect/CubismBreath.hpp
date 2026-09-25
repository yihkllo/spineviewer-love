

#pragma once

#include "Model/CubismModel.hpp"
#include "Id/CubismId.hpp"
#include "Type/csmVector.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

class CubismBreath
{
public:
    struct BreathParameterData
    {
        BreathParameterData()
                             : ParameterId(NULL)
                             , Offset(0.0f)
                             , Peak(0.0f)
                             , Cycle(0.0f)
                             , Weight(0.0f)
        { }

        BreathParameterData(CubismIdHandle parameterId, csmFloat32 offset, csmFloat32 peak, csmFloat32 cycle, csmFloat32 weight)
            : ParameterId(parameterId)
            , Offset(offset)
            , Peak(peak)
            , Cycle(cycle)
            , Weight(weight)
        { }

        CubismIdHandle ParameterId;
        csmFloat32 Offset;
        csmFloat32 Peak;
        csmFloat32 Cycle;
        csmFloat32 Weight;
    };

    static CubismBreath* Create();


    static void Delete(CubismBreath* instance);

    void SetParameters(const csmVector<BreathParameterData>& breathParameters);


    const csmVector<BreathParameterData>& GetParameters() const;


    void UpdateParameters(CubismModel* model, csmFloat32 deltaTimeSeconds);

private:
    CubismBreath();

    virtual ~CubismBreath();

    csmVector<BreathParameterData> _breathParameters;
    csmFloat32 _currentTime;
};

}}}
