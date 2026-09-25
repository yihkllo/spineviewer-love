
#pragma once

#include <MetalKit/MetalKit.h>
#include "../CubismRenderer.hpp"
#include "../CubismClippingManager.hpp"
#include "CubismFramework.hpp"
#include "CubismOffscreenSurface_Metal.hpp"
#include "CubismCommandBuffer_Metal.hpp"
#include "Type/csmVector.hpp"
#include "Type/csmRectF.hpp"
#include "Type/csmMap.hpp"
#include "Math/CubismVector2.hpp"

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

class CubismRenderer_Metal;
class CubismClippingContext_Metal;
class CubismShader_Metal;

class CubismClippingManager_Metal : public CubismClippingManager<CubismClippingContext_Metal, CubismOffscreenSurface_Metal>
{
public:

    void SetupClippingContext(CubismModel& model, CubismRenderer_Metal* renderer, CubismOffscreenSurface_Metal* lastColorBuffer, csmRectF lastViewport);
};

class CubismClippingContext_Metal : public CubismClippingContext
{
    friend class CubismClippingManager_Metal;
    friend class CubismShader_Metal;
    friend class CubismRenderer_Metal;

public:
    CubismClippingContext_Metal(CubismClippingManager<CubismClippingContext_Metal, CubismOffscreenSurface_Metal>* manager, CubismModel& model, const csmInt32* clippingDrawableIndices, csmInt32 clipCount);

    virtual ~CubismClippingContext_Metal();

    CubismClippingManager<CubismClippingContext_Metal, CubismOffscreenSurface_Metal>* GetClippingManager();

private:
    csmVector<CubismCommandBuffer_Metal::DrawCommandBuffer*>* _clippingCommandBufferList;
    CubismClippingManager<CubismClippingContext_Metal, CubismOffscreenSurface_Metal>* _owner;
};

class CubismRendererProfile_Metal
{
    friend class CubismRenderer_Metal;

private:
    CubismRendererProfile_Metal() {};

    virtual ~CubismRendererProfile_Metal() {};

    csmBool _lastScissorTest;
    csmBool _lastBlend;
    csmBool _lastStencilTest;
    csmBool _lastDepthTest;
    CubismOffscreenSurface_Metal* _lastColorBuffer;
    id <MTLTexture> _lastDepthBuffer;
    id <MTLTexture> _lastStencilBuffer;
    csmRectF _lastViewport;
};

class CubismRenderer_Metal : public CubismRenderer
{
    friend class CubismRenderer;
    friend class CubismClippingManager_Metal;
    friend class CubismShader_Metal;

public:

    static void StartFrame(id<MTLDevice> device, id<MTLCommandBuffer> commandBuffer, MTLRenderPassDescriptor* renderPassDescriptor);

    void Initialize(Framework::CubismModel* model) override;

    void Initialize(Framework::CubismModel* model, csmInt32 maskBufferCount) override;

    void BindTexture(csmUint32 modelTextureIndex, id <MTLTexture> texture);

    const csmMap< csmInt32, id <MTLTexture> >& GetBindedTextures() const;

    id <MTLTexture> GetBindedTextureId(csmInt32 textureId);

    void SetClippingMaskBufferSize(csmFloat32 width, csmFloat32 height);

    csmInt32 GetRenderTextureCount() const;

    CubismVector2 GetClippingMaskBufferSize() const;

    CubismOffscreenSurface_Metal* GetOffscreenSurface(csmInt32 index);

    CubismCommandBuffer_Metal* GetCommandBuffer()
    {
        return &_commandBuffer;
    }

protected:
    CubismRenderer_Metal();

    virtual ~CubismRenderer_Metal();

    void DoDrawModel() override;

    void DrawMeshMetal(CubismCommandBuffer_Metal::DrawCommandBuffer* drawCommandBuffer
                    , id <MTLRenderCommandEncoder> renderEncoder
                    , const CubismModel& model, const csmInt32 index);

    CubismCommandBuffer_Metal::DrawCommandBuffer* GetDrawCommandBufferData(csmInt32 drawableIndex);

private:

    CubismRenderer_Metal(const CubismRenderer_Metal&);
    CubismRenderer_Metal& operator=(const CubismRenderer_Metal&);

    static id<MTLCommandBuffer> s_commandBuffer;
    static id<MTLDevice> s_device;
    static MTLRenderPassDescriptor* s_renderPassDescriptor;

    static void DoStaticRelease();

    id <MTLRenderCommandEncoder> PreDraw(id <MTLCommandBuffer> commandBuffer, MTLRenderPassDescriptor* drawableRenderDescriptor);

    void PostDraw(id <MTLRenderCommandEncoder> renderEncoder);

    void SaveProfile() override;

    void RestoreProfile() override;

    void SetClippingContextBufferForMask(CubismClippingContext_Metal* clip);

    CubismClippingContext_Metal* GetClippingContextBufferForMask() const;

    void SetClippingContextBufferForDraw(CubismClippingContext_Metal* clip);

    CubismClippingContext_Metal* GetClippingContextBufferForDraw() const;

    const inline csmBool IsGeneratingMask() const;

    csmMap< csmInt32, id <MTLTexture> > _textures;
    csmVector<csmInt32> _sortedDrawableIndexList;
    CubismRendererProfile_Metal _rendererProfile;
    CubismClippingManager_Metal* _clippingManager;
    CubismClippingContext_Metal* _clippingContextBufferForMask;
    CubismClippingContext_Metal* _clippingContextBufferForDraw;

    csmVector<CubismOffscreenSurface_Metal> _offscreenSurfaces;
    CubismCommandBuffer_Metal _commandBuffer;
    csmVector<CubismCommandBuffer_Metal::DrawCommandBuffer*> _drawableDrawCommandBuffer;
};

}}}}
