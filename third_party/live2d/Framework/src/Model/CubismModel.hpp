

#pragma once

#include "CubismFramework.hpp"
#include "Type/csmMap.hpp"
#include "Type/csmVector.hpp"
#include "Rendering/CubismRenderer.hpp"
#include "Id/CubismId.hpp"

namespace Live2D { namespace Cubism { namespace Framework {

class CubismMoc;

class CubismModel
{
    friend class CubismMoc;
public:

    struct DrawableColorData
    {
        DrawableColorData()
            : IsOverwritten(false)
            , Color() {};

        DrawableColorData(csmBool isOverwritten, Rendering::CubismRenderer::CubismTextureColor color)
            : IsOverwritten(isOverwritten)
            , Color(color) {};

        virtual ~DrawableColorData() {};

        csmBool IsOverwritten;
        Rendering::CubismRenderer::CubismTextureColor Color;

    };

    struct DrawableCullingData
    {
        DrawableCullingData()
            : IsOverwritten(false)
            , IsCulling(0) {};

        DrawableCullingData(csmBool isOverwritten, csmInt32 isCulling)
            : IsOverwritten(isOverwritten)
            , IsCulling(isCulling) {};

        virtual ~DrawableCullingData() {};

        csmBool IsOverwritten;
        csmInt32 IsCulling;

    };

    struct PartColorData
    {
        PartColorData()
            : IsOverwritten(false)
            , Color() {};

        PartColorData(csmBool isOverwritten, Rendering::CubismRenderer::CubismTextureColor color)
            : IsOverwritten(isOverwritten)
            , Color(color) {};

        virtual ~PartColorData() {};

        csmBool IsOverwritten;
        Rendering::CubismRenderer::CubismTextureColor Color;

    };

    void    Update() const;

    csmFloat32  GetCanvasWidthPixel() const;

    csmFloat32  GetCanvasHeightPixel() const;

    csmFloat32  GetPixelsPerUnit() const;

    csmFloat32  GetCanvasWidth() const;

    csmFloat32  GetCanvasHeight() const;

    csmInt32    GetPartIndex(CubismIdHandle partId);

    CubismIdHandle    GetPartId(csmUint32 partIndex);

    csmInt32    GetPartCount() const;

    void        SetPartOpacity(CubismIdHandle partId, csmFloat32 opacity);

    void        SetPartOpacity(csmInt32 partIndex, csmFloat32 opacity);

    csmFloat32  GetPartOpacity(CubismIdHandle partId);

    csmFloat32  GetPartOpacity(csmInt32 partIndex);

    csmInt32    GetParameterIndex(CubismIdHandle parameterId);

    CubismIdHandle    GetParameterId(csmUint32 parameterIndex);

    csmInt32    GetParameterCount() const;

    Core::csmParameterType GetParameterType(csmUint32 parameterIndex) const;

    csmFloat32  GetParameterMaximumValue(csmUint32 parameterIndex) const;

    csmFloat32  GetParameterMinimumValue(csmUint32 parameterIndex) const;

    csmFloat32  GetParameterDefaultValue(csmUint32 parameterIndex) const;

    csmFloat32  GetParameterValue(CubismIdHandle parameterId);

    csmFloat32  GetParameterValue(csmInt32 parameterIndex);

    void        SetParameterValue(CubismIdHandle parameterId, csmFloat32 value, csmFloat32 weight = 1.0f);

    void        SetParameterValue(csmInt32 parameterIndex, csmFloat32 value, csmFloat32 weight = 1.0f);

    void        AddParameterValue(CubismIdHandle parameterId, csmFloat32 value, csmFloat32 weight = 1.0f);

    void        AddParameterValue(csmInt32 parameterIndex, csmFloat32 value, csmFloat32 weight = 1.0f);

    void        MultiplyParameterValue(CubismIdHandle parameterId, csmFloat32 value, csmFloat32 weight = 1.0f);

    void        MultiplyParameterValue(csmInt32 parameterIndex, csmFloat32 value, csmFloat32 weight = 1.0f);

    csmInt32            GetDrawableIndex(CubismIdHandle drawableId) const;

    csmInt32            GetDrawableCount() const;

    CubismIdHandle      GetDrawableId(csmInt32 drawableIndex) const;

    const csmInt32*     GetDrawableRenderOrders() const;

    csmInt32            GetDrawableTextureIndices(csmInt32 drawableIndex) const;

    csmInt32            GetDrawableTextureIndex(csmInt32 drawableIndex) const;

    csmInt32            GetDrawableVertexIndexCount(csmInt32 drawableIndex) const;

    csmInt32            GetDrawableVertexCount(csmInt32 drawableIndex) const;

    const csmFloat32*   GetDrawableVertices(csmInt32 drawableIndex) const;

    const csmUint16*            GetDrawableVertexIndices(csmInt32 drawableIndex) const;

    const Core::csmVector2*     GetDrawableVertexPositions(csmInt32 drawableIndex) const;

    const Core::csmVector2*     GetDrawableVertexUvs(csmInt32 drawableIndex) const;

    csmFloat32                  GetDrawableOpacity(csmInt32 drawableIndex) const;

    Core::csmVector4 GetDrawableMultiplyColor(csmInt32 drawableIndex) const;

    Core::csmVector4 GetDrawableScreenColor(csmInt32 drawableIndex) const;

    csmInt32 GetDrawableParentPartIndex(csmUint32 drawableIndex) const;

    Rendering::CubismRenderer::CubismBlendMode   GetDrawableBlendMode(csmInt32 drawableIndex) const;

    csmBool                    GetDrawableInvertedMask(csmInt32 drawableIndex) const;

    csmBool                  GetDrawableDynamicFlagIsVisible(csmInt32 drawableIndex) const;

    csmBool                  GetDrawableDynamicFlagVisibilityDidChange(csmInt32 drawableIndex) const;

    csmBool                  GetDrawableDynamicFlagOpacityDidChange(csmInt32 drawableIndex) const;

    csmBool                  GetDrawableDynamicFlagDrawOrderDidChange(csmInt32 drawableIndex) const;

    csmBool                  GetDrawableDynamicFlagRenderOrderDidChange(csmInt32 drawableIndex) const;

    csmBool                  GetDrawableDynamicFlagVertexPositionsDidChange(csmInt32 drawableIndex) const;

    csmBool                  GetDrawableDynamicFlagBlendColorDidChange(csmInt32 drawableIndex) const;

    const csmInt32**            GetDrawableMasks() const;

    const csmInt32*             GetDrawableMaskCounts() const;

    csmBool     IsUsingMasking() const;

    void    LoadParameters();

    void    SaveParameters();

    Rendering::CubismRenderer::CubismTextureColor GetMultiplyColor(csmInt32 drawableIndex) const;

    Rendering::CubismRenderer::CubismTextureColor GetScreenColor(csmInt32 drawableIndex) const;

    void SetMultiplyColor(csmInt32 drawableIndex, const Rendering::CubismRenderer::CubismTextureColor& color);

    void SetMultiplyColor(csmInt32 drawableIndex, csmFloat32 r, csmFloat32 g, csmFloat32 b, csmFloat32 a = 1.0f);

    void SetScreenColor(csmInt32 drawableIndex, const Rendering::CubismRenderer::CubismTextureColor& color);

    void SetScreenColor(csmInt32 drawableIndex, csmFloat32 r, csmFloat32 g, csmFloat32 b, csmFloat32 a = 1.0f);

    Rendering::CubismRenderer::CubismTextureColor GetPartMultiplyColor(csmInt32 partIndex) const;

    Rendering::CubismRenderer::CubismTextureColor GetPartScreenColor(csmInt32 partIndex) const;

    void SetPartMultiplyColor(csmInt32 partIndex, const Rendering::CubismRenderer::CubismTextureColor& color);

    void SetPartMultiplyColor(csmInt32 partIndex, csmFloat32 r, csmFloat32 g, csmFloat32 b, csmFloat32 a = 1.0f);

    void SetPartScreenColor(csmInt32 partIndex, const Rendering::CubismRenderer::CubismTextureColor& color);

    void SetPartScreenColor(csmInt32 partIndex, csmFloat32 r, csmFloat32 g, csmFloat32 b, csmFloat32 a = 1.0f);

    csmBool GetOverwriteFlagForModelMultiplyColors() const;

    csmBool GetOverwriteFlagForModelScreenColors() const;

    void SetOverwriteFlagForModelMultiplyColors(csmBool value);

    void SetOverwriteFlagForModelScreenColors(csmBool value);

    csmBool GetOverwriteFlagForDrawableMultiplyColors(csmInt32 drawableIndex) const;

    csmBool GetOverwriteFlagForDrawableScreenColors(csmInt32 drawableIndex) const;

    void SetOverwriteFlagForDrawableMultiplyColors(csmUint32 drawableIndex, csmBool value);

    void SetOverwriteFlagForDrawableScreenColors(csmUint32 drawableIndex, csmBool value);

    csmBool GetOverwriteColorForPartMultiplyColors(csmInt32 partIndex) const;

    csmBool GetOverwriteColorForPartScreenColors(csmInt32 partIndex) const;

    void SetOverwriteColorForPartMultiplyColors(csmUint32 partIndex, csmBool value);

    void SetOverwriteColorForPartScreenColors(csmUint32 partIndex, csmBool value);

    csmInt32 GetDrawableCulling(csmInt32 drawableIndex) const;

    void SetDrawableCulling(csmInt32 drawableIndex, csmInt32 isCulling);

    csmBool GetOverwriteFlagForModelCullings() const;

    void SetOverwriteFlagForModelCullings(csmBool value);

    csmBool GetOverwriteFlagForDrawableCullings(csmInt32 drawableIndex) const;

    void SetOverwriteFlagForDrawableCullings(csmUint32 drawableIndex, csmBool value);

    csmFloat32 GetModelOpacity();

    void SetModelOpacity(csmFloat32 value);

    Core::csmModel*     GetModel() const;

private:
    CubismModel(Core::csmModel* model);

    virtual ~CubismModel();

    CubismModel(const CubismModel&);
    CubismModel& operator=(const CubismModel&);

    void Initialize();

    void SetPartColor(
        csmUint32 partIndex,
        csmFloat32 r, csmFloat32 g, csmFloat32 b, csmFloat32 a,
        csmVector<PartColorData>& partColors,
        csmVector <DrawableColorData>& drawableColors);

    void SetOverwriteColorForPartColors(
        csmUint32 partIndex,
        csmBool value,
        csmVector<CubismModel::PartColorData>& partColors,
        csmVector <CubismModel::DrawableColorData>& drawableColors);

    csmMap<csmInt32, csmFloat32>        _notExistPartOpacities;
    csmMap<CubismIdHandle, csmInt32>   _notExistPartId;

    csmMap<csmInt32, csmFloat32>        _notExistParameterValues;
    csmMap<CubismIdHandle, csmInt32>   _notExistParameterId;

    csmVector<csmFloat32>   _savedParameters;

    Core::csmModel*     _model;

    csmFloat32*         _parameterValues;
    const csmFloat32*   _parameterMaximumValues;
    const csmFloat32*   _parameterMinimumValues;

    csmFloat32*         _partOpacities;

    csmFloat32 _modelOpacity;

    csmVector<CubismIdHandle> _parameterIds;
    csmVector<CubismIdHandle> _partIds;
    csmVector<CubismIdHandle> _drawableIds;
    csmVector<DrawableColorData> _userScreenColors;
    csmVector<DrawableColorData> _userMultiplyColors;
    csmVector<DrawableCullingData> _userCullings;
    csmVector<PartColorData> _userPartScreenColors;
    csmVector<PartColorData> _userPartMultiplyColors;
    csmVector<csmVector<csmUint32> > _partChildDrawables;
    csmBool _isOverwrittenModelMultiplyColors;
    csmBool _isOverwrittenModelScreenColors;
    csmBool _isOverwrittenCullings;
};

}}}
