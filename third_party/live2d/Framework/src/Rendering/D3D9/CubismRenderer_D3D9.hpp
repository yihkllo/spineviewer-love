

#pragma once

#include "CubismNativeInclude_D3D9.hpp"

#include "../CubismRenderer.hpp"
#include "../CubismClippingManager.hpp"
#include "CubismFramework.hpp"
#include "Type/csmVector.hpp"
#include "Type/csmRectF.hpp"
#include "Math/CubismVector2.hpp"
#include "Type/csmMap.hpp"
#include "Rendering/D3D9/CubismOffscreenSurface_D3D9.hpp"
#include "CubismRenderState_D3D9.hpp"
#include "CubismType_D3D9.hpp"

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

class CubismRenderer_D3D9;
class CubismShader_D3D9;
class CubismClippingContext_D3D9;

D3DXMATRIX ConvertToD3DX(CubismMatrix44& mtx);

class CubismClippingManager_DX9 : public CubismClippingManager<CubismClippingContext_D3D9, CubismOffscreenSurface_D3D9>
{
public:
    void SetupClippingContext(LPDIRECT3DDEVICE9 device, CubismModel& model, CubismRenderer_D3D9* renderer, csmInt32 offscreenCurrent);
};

class CubismClippingContext_D3D9 : public CubismClippingContext
{
    friend class CubismClippingManager_DX9;
    friend class CubismShader_D3D9;
    friend class CubismRenderer_D3D9;

public:
    CubismClippingContext_D3D9(CubismClippingManager<CubismClippingContext_D3D9, CubismOffscreenSurface_D3D9>* manager, CubismModel& model, const csmInt32* clippingDrawableIndices, csmInt32 clipCount);

    virtual ~CubismClippingContext_D3D9();

    CubismClippingManager<CubismClippingContext_D3D9, CubismOffscreenSurface_D3D9>* GetClippingManager();

private:
    CubismClippingManager<CubismClippingContext_D3D9, CubismOffscreenSurface_D3D9>* _owner;
};


class CubismRenderer_D3D9 : public CubismRenderer
{
    friend class CubismRenderer;
    friend class CubismClippingManager_DX9;
    friend class CubismShader_D3D9;

public:

    static void InitializeConstantSettings(csmUint32 bufferSetNum, LPDIRECT3DDEVICE9 device);

    static void SetDefaultRenderState();

    static void StartFrame(LPDIRECT3DDEVICE9 device, csmUint32 viewportWidth, csmUint32 viewportHeight);

    static void EndFrame(LPDIRECT3DDEVICE9 device);

    static CubismRenderState_D3D9* GetRenderStateManager();

    static void DeleteRenderStateManager();

    static CubismShader_D3D9* GetShaderManager();

    static void DeleteShaderManager();

    static void OnDeviceLost();

    static void GenerateShader(LPDIRECT3DDEVICE9 device);

    virtual void CubismRenderer_D3D9::Initialize(CubismModel* model);

    virtual void Initialize(Framework::CubismModel* model, csmInt32 maskBufferCount) override;

    void FrameRenderingInit();

    void BindTexture(csmUint32 modelTextureAssign, const LPDIRECT3DTEXTURE9 texture);

    const csmMap<csmInt32, LPDIRECT3DTEXTURE9>& GetBindedTextures() const;

    void SetClippingMaskBufferSize(csmFloat32 width, csmFloat32 height);

    csmInt32 GetRenderTextureCount() const;

    CubismVector2 GetClippingMaskBufferSize() const;

    CubismOffscreenSurface_D3D9* GetMaskBuffer(csmUint32 backbufferNum, csmInt32 offscreenIndex);

protected:
    CubismRenderer_D3D9();

    virtual ~CubismRenderer_D3D9();

    virtual void DoDrawModel() override;

    void DrawMeshDX9(const CubismModel& model, const csmInt32 index);

    void ExecuteDrawForDraw(const CubismModel& model, const csmInt32 index);

    void ExecuteDrawForMask(const CubismModel& model, const csmInt32 index);

private:

    static void DoStaticRelease();

    static void ReleaseShader();

    CubismRenderer_D3D9(const CubismRenderer_D3D9&);
    CubismRenderer_D3D9& operator=(const CubismRenderer_D3D9&);

    void PreDraw();

    void PostDraw();

    virtual void SaveProfile() override;

    virtual void RestoreProfile() override;

    void SetClippingContextBufferForMask(CubismClippingContext_D3D9* clip);

    CubismClippingContext_D3D9* GetClippingContextBufferForMask() const;

    void SetClippingContextBufferForDraw(CubismClippingContext_D3D9* clip);

    CubismClippingContext_D3D9* GetClippingContextBufferForDraw() const;

    void CopyToBuffer(csmInt32 drawAssign, const csmInt32 vcount, const csmFloat32* varray, const csmFloat32* uvarray);

    LPDIRECT3DTEXTURE9 GetTextureWithIndex(const CubismModel& model, const csmInt32 index);

    const csmBool inline IsGeneratingMask() const;

    void SetBlendMode(CubismBlendMode blendMode) const;

    void SetTechniqueForDraw(const CubismModel& model, const csmInt32 index) const;

    void SetTextureFilter() const;

    void SetColorVectors(ID3DXEffect* shaderEffect, CubismTextureColor& baseColor, CubismTextureColor& multiplyColor, CubismTextureColor& screenColor);

    void SetExecutionTextures(const CubismModel& model, const csmInt32 index, ID3DXEffect* shaderEffect);

    void SetColorChannel(ID3DXEffect* shaderEffect, CubismClippingContext_D3D9* contextBuffer);

    void SetProjectionMatrix(ID3DXEffect* shaderEffect, CubismMatrix44& matrix);

    void DrawIndexedPrimiteveWithSetup(const CubismModel& model, const csmInt32 index);

    csmUint32 _drawableNum;

    CubismVertexD3D9** _vertexStore;
    csmUint16** _indexStore;

    csmInt32 _commandBufferNum;
    csmInt32 _commandBufferCurrent;

    csmVector<csmInt32> _sortedDrawableIndexList;

    csmMap<csmInt32, LPDIRECT3DTEXTURE9> _textures;

    csmVector<csmVector<CubismOffscreenSurface_D3D9> > _offscreenSurfaces;

    CubismClippingManager_DX9* _clippingManager;
    CubismClippingContext_D3D9* _clippingContextBufferForMask;
    CubismClippingContext_D3D9* _clippingContextBufferForDraw;
};

}}}}
