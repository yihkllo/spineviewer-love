

#pragma once

#include <float.h>
#include "CubismFramework.hpp"
#include "Type/csmVector.hpp"
#include "Type/csmRectF.hpp"
#include "Math/CubismVector2.hpp"
#include "Math/CubismMatrix44.hpp"
#include "Model/CubismModel.hpp"

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

#ifndef CSM_MASK_FILESCOPE
#define CSM_MASK_FILESCOPE
namespace {
const csmInt32 ColorChannelCount = 4;
const csmInt32 ClippingMaskMaxCountOnDefault = 36;
const csmInt32 ClippingMaskMaxCountOnMultiRenderTexture = 32;
}
#endif

template <class T_ClippingContext, class T_OffscreenSurface>
class CubismClippingManager
{
public:
    CubismClippingManager();

    virtual ~CubismClippingManager();

    void Initialize(CubismModel& model, const csmInt32 maskBufferCount);

    T_ClippingContext* FindSameClip(const csmInt32* drawableMasks, csmInt32 drawableMaskCounts) const;

    void SetupMatrixForHighPrecision(CubismModel& model, csmBool isRightHanded);

    void createMatrixForMask(csmBool isRightHanded, csmRectF* layoutBoundsOnTex01, csmFloat32 scaleX, csmFloat32 scaleY);

    void SetupLayoutBounds(csmInt32 usingClipCount) const;

    void CalcClippedDrawTotalBounds(CubismModel& model, T_ClippingContext* clippingContext);

    csmVector<T_ClippingContext*>* GetClippingContextListForDraw();

    CubismVector2 GetClippingMaskBufferSize() const;

    csmInt32 GetRenderTextureCount();

    CubismRenderer::CubismTextureColor* GetChannelFlagAsColor(csmInt32 channelIndex);

    void SetClippingMaskBufferSize(csmFloat32 width, csmFloat32 height);

protected:
    T_OffscreenSurface* _currentMaskBuffer;
    csmVector<csmBool> _clearedMaskBufferFlags;

    csmVector<CubismRenderer::CubismTextureColor*> _channelColors;
    csmVector<T_ClippingContext*> _clippingContextListForMask;
    csmVector<T_ClippingContext*> _clippingContextListForDraw;
    CubismVector2 _clippingMaskBufferSize;
    csmInt32 _renderTextureCount;

    CubismMatrix44 _tmpMatrix;
    CubismMatrix44 _tmpMatrixForMask;
    CubismMatrix44 _tmpMatrixForDraw;
    csmRectF _tmpBoundsOnModel;
};

#include "CubismClippingManager.tpp"
}}}}

