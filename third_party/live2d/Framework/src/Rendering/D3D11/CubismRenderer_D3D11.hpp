

#pragma once

#include "CubismNativeInclude_D3D11.hpp"

#include "../CubismRenderer.hpp"
#include "../CubismClippingManager.hpp"
#include "CubismFramework.hpp"
#include "CubismType_D3D11.hpp"
#include "Type/csmVector.hpp"
#include "Type/csmRectF.hpp"
#include "Math/CubismVector2.hpp"
#include "Type/csmMap.hpp"
#include "Rendering/D3D11/CubismOffscreenSurface_D3D11.hpp"
#include "CubismRenderState_D3D11.hpp"
#include <unordered_set>

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

class CubismRenderer_D3D11;
class CubismShader_D3D11;
class CubismClippingContext_D3D11;

DirectX::XMMATRIX ConvertToD3DX(CubismMatrix44& mtx);

class CubismClippingManager_D3D11 : public CubismClippingManager<CubismClippingContext_D3D11, CubismOffscreenSurface_D3D11>
{
public:

    void SetupClippingContext(ID3D11Device* device, ID3D11DeviceContext* renderContext, CubismModel& model, CubismRenderer_D3D11* renderer, csmInt32 offscreenCurrent);
};


class CubismClippingContext_D3D11 : public CubismClippingContext
{
    friend class CubismClippingManager_D3D11;
    friend class CubismShader_D3D11;
    friend class CubismRenderer_D3D11;

public:
    CubismClippingContext_D3D11(CubismClippingManager<CubismClippingContext_D3D11, CubismOffscreenSurface_D3D11>* manager, CubismModel& model, const csmInt32* clippingDrawableIndices, csmInt32 clipCount);

    virtual ~CubismClippingContext_D3D11();

    CubismClippingManager<CubismClippingContext_D3D11, CubismOffscreenSurface_D3D11>* GetClippingManager();

    CubismClippingManager<CubismClippingContext_D3D11, CubismOffscreenSurface_D3D11>* _owner;
};


class CubismRenderer_D3D11 : public CubismRenderer
{
    friend class CubismRenderer;
    friend class CubismClippingManager_D3D11;
    friend class CubismShader_D3D11;

public:
    void SetDrawableDisabled(csmInt32 index) { _disabledDrawables.insert(index); }

    static void InitializeConstantSettings(csmUint32 bufferSetNum, ID3D11Device* device);

    static void SetDefaultRenderState();

    static void StartFrame(ID3D11Device* device, ID3D11DeviceContext* renderContext, csmUint32 viewportWidth, csmUint32 viewportHeight);

    static void EndFrame(ID3D11Device* device);

    static CubismRenderState_D3D11* GetRenderStateManager();

    static void DeleteRenderStateManager();

    static CubismShader_D3D11* GetShaderManager();

    static void DeleteShaderManager();

    static void OnDeviceLost();

    static void GenerateShader(ID3D11Device* device);

    static ID3D11Device* GetCurrentDevice();

    virtual void Initialize(Framework::CubismModel* model);

    virtual void Initialize(Framework::CubismModel* model, csmInt32 maskBufferCount);

    void BindTexture(csmUint32 modelTextureAssign, ID3D11ShaderResourceView* textureView);

    const csmMap<csmInt32, ID3D11ShaderResourceView*>& GetBindedTextures() const;

    void SetClippingMaskBufferSize(csmFloat32 width, csmFloat32 height);

    csmInt32 GetRenderTextureCount() const;

    CubismVector2 GetClippingMaskBufferSize() const;

    CubismOffscreenSurface_D3D11* GetMaskBuffer(csmUint32 backbufferNum, csmInt32 offscreenIndex);

protected:
    CubismRenderer_D3D11();

    virtual ~CubismRenderer_D3D11();

    virtual void DoDrawModel() override;

    void DrawMeshDX11(const CubismModel& model, const csmInt32 index);

private:
    std::unordered_set<csmInt32> _disabledDrawables;

    void ExecuteDrawForMask(const CubismModel& model, const csmInt32 index);

    void ExecuteDrawForDraw(const CubismModel& model, const csmInt32 index);

    void DrawDrawableIndexed(const CubismModel& model, const csmInt32 index);

    static void DoStaticRelease();

    static void ReleaseShader();


    CubismRenderer_D3D11(const CubismRenderer_D3D11&);
    CubismRenderer_D3D11& operator=(const CubismRenderer_D3D11&);

    void PreDraw();

    void PostDraw();

    virtual void SaveProfile() override;

    virtual void RestoreProfile() override;

    void SetClippingContextBufferForMask(CubismClippingContext_D3D11* clip);

    CubismClippingContext_D3D11* GetClippingContextBufferForMask() const;

    void SetClippingContextBufferForDraw(CubismClippingContext_D3D11* clip);

    CubismClippingContext_D3D11* GetClippingContextBufferForDraw() const;

    void CopyToBuffer(ID3D11DeviceContext* renderContext, csmInt32 drawAssign, const csmInt32 vcount, const csmFloat32* varray, const csmFloat32* uvarray);

    ID3D11ShaderResourceView* GetTextureViewWithIndex(const CubismModel& model, const csmInt32 index);

    void SetBlendState(const CubismBlendMode blendMode);

    void SetShader(const CubismModel& model, const csmInt32 index);

    void SetTextureView(const CubismModel& model, const csmInt32 index);

    void SetColorConstantBuffer(CubismConstantBufferD3D11& cb, const CubismModel& model, const csmInt32 index,
                                CubismTextureColor& baseColor, CubismTextureColor& multiplyColor, CubismTextureColor& screenColor);

    void SetColorChannel(CubismConstantBufferD3D11& cb, CubismClippingContext_D3D11* contextBuffer);

    void SetProjectionMatrix(CubismConstantBufferD3D11& cb, CubismMatrix44 matrix);

    void UpdateConstantBuffer(CubismConstantBufferD3D11& cb, csmInt32 index);

    void SetSamplerAccordingToAnisotropy();

    const csmBool inline IsGeneratingMask() const;

    ID3D11Buffer*** _vertexBuffers;
    ID3D11Buffer*** _indexBuffers;
    ID3D11Buffer*** _constantBuffers;
    csmUint32 _drawableNum;

    csmInt32 _commandBufferNum;
    csmInt32 _commandBufferCurrent;

    csmVector<csmInt32> _sortedDrawableIndexList;

    csmMap<csmInt32, ID3D11ShaderResourceView*> _textures;

    csmVector<csmVector<CubismOffscreenSurface_D3D11> > _offscreenSurfaces;

    CubismClippingManager_D3D11* _clippingManager;
    CubismClippingContext_D3D11* _clippingContextBufferForMask;
    CubismClippingContext_D3D11* _clippingContextBufferForDraw;
};

}}}}
