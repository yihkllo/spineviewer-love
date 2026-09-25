

#pragma once

#include "CubismFramework.hpp"
#include "Math/CubismMatrix44.hpp"
#include "Type/csmVector.hpp"
#include "Type/csmRectF.hpp"

namespace Live2D {namespace Cubism {namespace Framework {
class CubismModel;
}}}

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

class CubismRenderer
{
public:

    enum CubismBlendMode
    {
        CubismBlendMode_Normal = 0,

        CubismBlendMode_Additive = 1,

        CubismBlendMode_Multiplicative = 2,

        CubismBlendMode_Mask = 3,
    };

    struct CubismTextureColor
    {
        CubismTextureColor()
            : R(1.0f)
            , G(1.0f)
            , B(1.0f)
            , A(1.0f) {};

        CubismTextureColor(csmFloat32 r, csmFloat32 g, csmFloat32 b, csmFloat32 a)
            : R(r)
            , G(g)
            , B(b)
            , A(a) {};

        virtual ~CubismTextureColor() {};

        csmFloat32 R;
        csmFloat32 G;
        csmFloat32 B;
        csmFloat32 A;

    };

    static CubismRenderer* Create();

    static void Delete(CubismRenderer* renderer);

    static void StaticRelease();

    virtual void Initialize(Framework::CubismModel* model);

    virtual void Initialize(Framework::CubismModel* model, csmInt32 maskBufferCount);

    void DrawModel();

    void SetMvpMatrix(CubismMatrix44* matrix4x4);

    CubismMatrix44 GetMvpMatrix() const;

    void SetModelColor(csmFloat32 red, csmFloat32 green, csmFloat32 blue, csmFloat32 alpha);

    CubismTextureColor GetModelColor() const;

    CubismTextureColor GetModelColorWithOpacity(const csmFloat32 opacity) const;

    void IsPremultipliedAlpha(csmBool enable);

    csmBool IsPremultipliedAlpha() const;

    void IsCulling(csmBool culling);

    csmBool IsCulling() const;

    void SetAnisotropy(csmFloat32 anisotropy);

    csmFloat32 GetAnisotropy() const;

    CubismModel* GetModel() const;

    void UseHighPrecisionMask(csmBool high);

    csmBool IsUsingHighPrecisionMask();

protected:
    CubismRenderer();

    virtual ~CubismRenderer();

    virtual void DoDrawModel() = 0;

    virtual void SaveProfile() = 0;


    virtual void RestoreProfile() = 0;

private:
    CubismRenderer(const CubismRenderer&);
    CubismRenderer& operator=(const CubismRenderer&);

    CubismMatrix44      _mvpMatrix4x4;
    CubismTextureColor  _modelColor;
    csmBool             _isCulling;
    csmBool             _isPremultipliedAlpha;
    csmFloat32          _anisotropy;
    CubismModel*        _model;

    csmBool             _useHighPrecisionMask;
};


class CubismClippingContext
{
public:
    CubismClippingContext(const csmInt32* clippingDrawableIndices, csmInt32 clipCount);

    ~CubismClippingContext();

    void AddClippedDrawable(csmInt32 drawableIndex);

    csmBool _isUsing;
    const csmInt32* _clippingIdList;
    csmInt32 _clippingIdCount;
    csmInt32 _layoutChannelIndex;
    csmRectF* _layoutBounds;
    csmRectF* _allClippedDrawRect;
    CubismMatrix44 _matrixForMask;
    CubismMatrix44 _matrixForDraw;
    csmVector<csmInt32>* _clippedDrawableIndexList;
    csmInt32 _bufferIndex;
};

}}}}

