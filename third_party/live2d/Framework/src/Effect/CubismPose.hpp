

#pragma once

#include "Model/CubismModel.hpp"
#include "Utils/CubismJson.hpp"

namespace Live2D { namespace Cubism { namespace Framework {
class CubismPose
{
public:
    struct PartData
    {
        PartData();

        PartData(const PartData& v);

        virtual ~PartData();

        PartData&   operator=(const PartData& v);

        void                Initialize(CubismModel* model);

        CubismIdHandle                     PartId;
        csmInt32                            ParameterIndex;
        csmInt32                            PartIndex;
        csmVector<PartData>                 Link;
    };

    static CubismPose*  Create(const csmByte* pose3json, csmSizeInt size);

    static void         Delete(CubismPose* pose);

    void                UpdateParameters(CubismModel* model, csmFloat32 deltaTimeSeconds);

    void                Reset(CubismModel* model);

private:
    CubismPose();

    virtual ~CubismPose();

    void                CopyPartOpacities(CubismModel* model);

    void                DoFade(CubismModel* model, csmFloat32 deltaTimeSeconds, csmInt32 beginIndex, csmInt32 partGroupCount);

    csmVector<PartData>             _partGroups;
    csmVector<csmInt32>             _partGroupCounts;
    csmFloat32                      _fadeTimeSeconds;
    CubismModel*                    _lastModel;
};

}}}
