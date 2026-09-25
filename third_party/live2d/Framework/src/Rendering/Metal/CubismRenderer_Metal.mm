
#include "CubismRenderer_Metal.hpp"
#include "Math/CubismMatrix44.hpp"
#include "Model/CubismModel.hpp"
#include "CubismShader_Metal.hpp"
#include "CubismRenderingInstanceSingleton_Metal.h"
#include "MetalShaderTypes.h"

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

void CubismClippingManager_Metal::SetupClippingContext(CubismModel& model, CubismRenderer_Metal* renderer, CubismOffscreenSurface_Metal* lastColorBuffer, csmRectF lastViewport)
{
    csmInt32 usingClipCount = 0;
    for (csmUint32 clipIndex = 0; clipIndex < _clippingContextListForMask.GetSize(); clipIndex++)
    {
        CubismClippingContext_Metal* cc = _clippingContextListForMask[clipIndex];

        CalcClippedDrawTotalBounds(model, cc);

        if (cc->_isUsing)
        {
            usingClipCount++;
        }
    }

    if (usingClipCount <= 0)
    {
        return;
    }

    id <MTLRenderCommandEncoder> renderEncoder = nil;
    MTLViewport clipVp = {0, 0, GetClippingMaskBufferSize().X, GetClippingMaskBufferSize().Y, 0.0, 1.0};

    _currentMaskBuffer = renderer->GetOffscreenSurface(0);
    renderEncoder = renderer->PreDraw(renderer->s_commandBuffer, _currentMaskBuffer->GetRenderPassDescriptor());

    SetupLayoutBounds(usingClipCount);

    if (_clearedMaskBufferFlags.GetSize() != _renderTextureCount)
    {
        _clearedMaskBufferFlags.Clear();

        for (csmInt32 i = 0; i < _renderTextureCount; ++i)
        {
            _clearedMaskBufferFlags.PushBack(false);
        }
    }
    else
    {
        for (csmInt32 i = 0; i < _renderTextureCount; ++i)
        {
            _clearedMaskBufferFlags[i] = false;
        }
    }

    for (csmUint32 clipIndex = 0; clipIndex < _clippingContextListForMask.GetSize(); clipIndex++)
    {
        CubismClippingContext_Metal* clipContext = _clippingContextListForMask[clipIndex];
        csmRectF* allClippedDrawRect = clipContext->_allClippedDrawRect;
        csmRectF* layoutBoundsOnTex01 = clipContext->_layoutBounds;
        const csmFloat32 MARGIN = 0.05f;
        const csmBool isRightHanded = false;

        CubismOffscreenSurface_Metal* clipContextOffscreenSurface = renderer->GetOffscreenSurface(clipContext->_bufferIndex);

        if (_currentMaskBuffer != clipContextOffscreenSurface)
        {
            _currentMaskBuffer = clipContextOffscreenSurface;

            [renderEncoder endEncoding];
            renderEncoder = renderer->PreDraw(renderer->s_commandBuffer, _currentMaskBuffer->GetRenderPassDescriptor());
        }

        _tmpBoundsOnModel.SetRect(allClippedDrawRect);
        _tmpBoundsOnModel.Expand(allClippedDrawRect->Width * MARGIN, allClippedDrawRect->Height * MARGIN);
        csmFloat32 scaleX = layoutBoundsOnTex01->Width / _tmpBoundsOnModel.Width;
        csmFloat32 scaleY = layoutBoundsOnTex01->Height / _tmpBoundsOnModel.Height;

        createMatrixForMask(isRightHanded, layoutBoundsOnTex01, scaleX, scaleY);

        clipContext->_matrixForMask.SetMatrix(_tmpMatrixForMask.GetArray());
        clipContext->_matrixForDraw.SetMatrix(_tmpMatrixForDraw.GetArray());

        const csmInt32 clipDrawCount = clipContext->_clippingIdCount;
        for (csmInt32 i = 0; i < clipDrawCount; i++)
        {
            const csmInt32 clipDrawIndex = clipContext->_clippingIdList[i];
            CubismCommandBuffer_Metal::DrawCommandBuffer* drawCommandBufferData = clipContext->_clippingCommandBufferList->At(i);

            if (!model.GetDrawableDynamicFlagVertexPositionsDidChange(clipDrawIndex))
            {
                continue;
            }

            {
                csmFloat32* vertices = const_cast<csmFloat32*>(model.GetDrawableVertices(clipDrawIndex));
                Core::csmVector2* uvs = const_cast<Core::csmVector2*>(model.GetDrawableVertexUvs(clipDrawIndex));
                csmUint16* vertexIndices = const_cast<csmUint16*>(model.GetDrawableVertexIndices(clipDrawIndex));
                const csmUint32 vertexCount = model.GetDrawableVertexCount(clipDrawIndex);
                const csmUint32 vertexIndexCount = model.GetDrawableVertexIndexCount(clipDrawIndex);

                drawCommandBufferData->UpdateVertexBuffer(vertices, uvs, vertexCount);
                if (vertexIndexCount > 0)
                {
                    drawCommandBufferData->UpdateIndexBuffer(vertexIndices, vertexIndexCount);
                }

                if (vertexCount <= 0)
                {
                    continue;
                }
            }

            renderer->IsCulling(model.GetDrawableCulling(clipDrawIndex) != 0);

            if (!_clearedMaskBufferFlags[clipContext->_bufferIndex])
            {
                [renderEncoder setViewport:clipVp];
                _clearedMaskBufferFlags[clipContext->_bufferIndex] = true;
            }

            renderer->SetClippingContextBufferForMask(clipContext);

            renderer->DrawMeshMetal(drawCommandBufferData, renderEncoder, model, clipDrawIndex);
        }
    }

    [renderEncoder endEncoding];
    renderer->SetClippingContextBufferForMask(NULL);
}

CubismClippingContext_Metal::CubismClippingContext_Metal(CubismClippingManager<CubismClippingContext_Metal, CubismOffscreenSurface_Metal>* manager, CubismModel& model, const csmInt32* clippingDrawableIndices, csmInt32 clipCount)
    : CubismClippingContext(clippingDrawableIndices, clipCount)
{
    CubismRenderingInstanceSingleton_Metal *single = [CubismRenderingInstanceSingleton_Metal sharedManager];
    id <MTLDevice> device = [single getMTLDevice];
    CAMetalLayer* metalLayer = [single getMetalLayer];

    _owner = manager;

    _clippingCommandBufferList = CSM_NEW csmVector<CubismCommandBuffer_Metal::DrawCommandBuffer*>;
    for (csmUint32 i = 0; i < _clippingIdCount; ++i)
    {
        const csmInt32 clippingId = _clippingIdList[i];
        CubismCommandBuffer_Metal::DrawCommandBuffer* drawCommandBuffer = NULL;
        const csmInt32 drawableVertexCount = model.GetDrawableVertexCount(clippingId);
        const csmInt32 drawableVertexIndexCount = model.GetDrawableVertexIndexCount(clippingId);
        const csmSizeInt vertexSize = sizeof(csmFloat32) * 2;


        drawCommandBuffer = CSM_NEW CubismCommandBuffer_Metal::DrawCommandBuffer();
        drawCommandBuffer->CreateVertexBuffer(device, vertexSize, drawableVertexCount * 2);
        drawCommandBuffer->CreateIndexBuffer(device, drawableVertexIndexCount);


        _clippingCommandBufferList->PushBack(drawCommandBuffer);
    }
}

CubismClippingContext_Metal::~CubismClippingContext_Metal()
{
    if (_clippingCommandBufferList != NULL)
    {
        for (csmUint32 i = 0; i < _clippingCommandBufferList->GetSize(); ++i)
        {
            if (_clippingCommandBufferList->At(i) != NULL)
            {
                CSM_DELETE(_clippingCommandBufferList->At(i));
                _clippingCommandBufferList->At(i) = NULL;
            }
        }

        CSM_DELETE(_clippingCommandBufferList);
        _clippingCommandBufferList = NULL;
    }
}

CubismClippingManager<CubismClippingContext_Metal, CubismOffscreenSurface_Metal>* CubismClippingContext_Metal::GetClippingManager()
{
    return _owner;
}


id<MTLCommandBuffer> CubismRenderer_Metal::s_commandBuffer = nil;
id<MTLDevice> CubismRenderer_Metal::s_device = nil;
MTLRenderPassDescriptor* CubismRenderer_Metal::s_renderPassDescriptor = nil;

CubismRenderer* CubismRenderer::Create()
{
    return CSM_NEW CubismRenderer_Metal();
}

void CubismRenderer::StaticRelease()
{
    CubismRenderer_Metal::DoStaticRelease();
}

CubismRenderer_Metal::CubismRenderer_Metal() : _clippingManager(NULL)
                                                     , _clippingContextBufferForMask(NULL)
                                                     , _clippingContextBufferForDraw(NULL)
{
    _textures.PrepareCapacity(32, true);
}

CubismRenderer_Metal::~CubismRenderer_Metal()
{
    CSM_DELETE_SELF(CubismClippingManager_Metal, _clippingManager);

    if (_drawableDrawCommandBuffer.GetSize() > 0)
    {
        for (csmInt32 i = 0; i < _drawableDrawCommandBuffer.GetSize(); ++i)
        {
            if (_drawableDrawCommandBuffer[i] != NULL)
            {
                CSM_DELETE(_drawableDrawCommandBuffer[i]);
            }
        }

        _drawableDrawCommandBuffer.Clear();
    }

    if (_textures.GetSize() > 0)
    {
        _textures.Clear();
    }

    for (csmUint32 i = 0; i < _offscreenSurfaces.GetSize(); ++i)
    {
        _offscreenSurfaces[i].DestroyOffscreenSurface();
    }
    _offscreenSurfaces.Clear();
}

void CubismRenderer_Metal::DoStaticRelease()
{
    s_commandBuffer = nil;
    CubismShader_Metal::DeleteInstance();
}

void CubismRenderer_Metal::Initialize(CubismModel* model)
{
    Initialize(model, 1);
}

void CubismRenderer_Metal::Initialize(CubismModel* model, csmInt32 maskBufferCount)
{
    if (maskBufferCount < 1)
    {
        maskBufferCount = 1;
        CubismLogWarning("The number of render textures must be an integer greater than or equal to 1. Set the number of render textures to 1.");
    }

    CubismRenderingInstanceSingleton_Metal *single = [CubismRenderingInstanceSingleton_Metal sharedManager];
    id <MTLDevice> device = [single getMTLDevice];
    CAMetalLayer* metalLayer = [single getMetalLayer];

    if (model->IsUsingMasking())
    {
        _clippingManager = CSM_NEW CubismClippingManager_Metal();
        _clippingManager->Initialize(
            *model,
            maskBufferCount
        );

        _offscreenSurfaces.Clear();

        for (csmInt32 i = 0; i < maskBufferCount; ++i)
        {
            CubismOffscreenSurface_Metal offscreenSurface;
            offscreenSurface.CreateOffscreenSurface(_clippingManager->GetClippingMaskBufferSize().X, _clippingManager->GetClippingMaskBufferSize().Y);
            _offscreenSurfaces.PushBack(offscreenSurface);
        }
    }

    _sortedDrawableIndexList.Resize(model->GetDrawableCount(), 0);

    _drawableDrawCommandBuffer.Resize(model->GetDrawableCount());

    for (csmInt32 i = 0; i < _drawableDrawCommandBuffer.GetSize(); ++i)
    {
        const csmInt32 drawableVertexCount = model->GetDrawableVertexCount(i);
        const csmInt32 drawableVertexIndexCount = model->GetDrawableVertexIndexCount(i);
        const csmSizeInt vertexSize = sizeof(csmFloat32) * 2;

        _drawableDrawCommandBuffer[i] = CSM_NEW CubismCommandBuffer_Metal::DrawCommandBuffer();

        _drawableDrawCommandBuffer[i]->CreateVertexBuffer(device, vertexSize, drawableVertexCount);

        if (drawableVertexIndexCount > 0)
        {
            _drawableDrawCommandBuffer[i]->CreateIndexBuffer(device, drawableVertexIndexCount);
        }
    }

    CubismRenderer::Initialize(model, maskBufferCount);
}

void CubismRenderer_Metal::StartFrame(id<MTLDevice> device, id<MTLCommandBuffer> commandBuffer, MTLRenderPassDescriptor* renderPassDescriptor)
{
    s_commandBuffer = commandBuffer;
    s_device = device;
    s_renderPassDescriptor = renderPassDescriptor;
}

id <MTLRenderCommandEncoder> CubismRenderer_Metal::PreDraw(id <MTLCommandBuffer> commandBuffer, MTLRenderPassDescriptor* drawableRenderDescriptor)
{
    id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:drawableRenderDescriptor];
    return renderEncoder;
}

void CubismRenderer_Metal::PostDraw(id <MTLRenderCommandEncoder> renderEncoder)
{
    [renderEncoder endEncoding];
}

void CubismRenderer_Metal::DoDrawModel()
{
    if (_clippingManager != NULL)
    {
        for (csmInt32 i = 0; i < _clippingManager->GetRenderTextureCount(); ++i)
        {
            if (_offscreenSurfaces[i].GetBufferWidth() != _clippingManager->GetClippingMaskBufferSize().X ||
                _offscreenSurfaces[i].GetBufferHeight() != _clippingManager->GetClippingMaskBufferSize().Y)
            {
                _offscreenSurfaces[i].CreateOffscreenSurface(
                    _clippingManager->GetClippingMaskBufferSize().X,
                    _clippingManager->GetClippingMaskBufferSize().Y
                );
            }
        }

        if (IsUsingHighPrecisionMask())
        {
           _clippingManager->SetupMatrixForHighPrecision(*GetModel(), false);
        }
        else
        {
            _clippingManager->SetupClippingContext(*GetModel(), this, _rendererProfile._lastColorBuffer, _rendererProfile._lastViewport);
        }
    }

    id <MTLRenderCommandEncoder> renderEncoder = nil;

    if(!IsUsingHighPrecisionMask())
    {
        renderEncoder = PreDraw(s_commandBuffer, s_renderPassDescriptor);
    }

    const csmInt32 drawableCount = GetModel()->GetDrawableCount();
    const csmInt32* renderOrder = GetModel()->GetDrawableRenderOrders();

    for (csmInt32 i = 0; i < drawableCount; ++i)
    {
        const csmInt32 order = renderOrder[i];
        _sortedDrawableIndexList[order] = i;
    }

    for (csmInt32 i = 0; i < drawableCount; ++i)
    {
        csmFloat32* vertices = const_cast<csmFloat32*>(GetModel()->GetDrawableVertices(i));
        Core::csmVector2* uvs = const_cast<Core::csmVector2*>(GetModel()->GetDrawableVertexUvs(i));
        csmUint16* vertexIndices = const_cast<csmUint16*>(GetModel()->GetDrawableVertexIndices(i));
        const csmUint32 vertexCount = GetModel()->GetDrawableVertexCount(i);
        const csmUint32 vertexIndexCount = GetModel()->GetDrawableVertexIndexCount(i);

        _drawableDrawCommandBuffer[i]->UpdateVertexBuffer(vertices, uvs, vertexCount);
        if (vertexIndexCount > 0)
        {
            _drawableDrawCommandBuffer[i]->UpdateIndexBuffer(vertexIndices, vertexIndexCount);
        }
    }

    for (csmInt32 i = 0; i < drawableCount; ++i)
    {
        const csmInt32 drawableIndex = _sortedDrawableIndexList[i];

        if (!GetModel()->GetDrawableDynamicFlagIsVisible(drawableIndex))
        {
            continue;
        }

        CubismClippingContext_Metal* clipContext = (_clippingManager != NULL)
            ? (*_clippingManager->GetClippingContextListForDraw())[drawableIndex]
            : NULL;

        if (clipContext != NULL && IsUsingHighPrecisionMask())
        {
            MTLViewport clipVp = {0, 0, _clippingManager->GetClippingMaskBufferSize().X, _clippingManager->GetClippingMaskBufferSize().Y, 0.0, 1.0};
            if(clipContext->_isUsing)
            {
                renderEncoder = PreDraw(s_commandBuffer, _offscreenSurfaces[ clipContext->_bufferIndex].GetRenderPassDescriptor());
                [renderEncoder setViewport:clipVp];
            }

            {
                const csmInt32 clipDrawCount = clipContext->_clippingIdCount;
                for (csmInt32 index = 0; index < clipDrawCount; index++)
                {
                    const csmInt32 clipDrawIndex = clipContext->_clippingIdList[index];
                    CubismCommandBuffer_Metal::DrawCommandBuffer::DrawCommand* drawCommandMask = clipContext->_clippingCommandBufferList->At(index)->GetCommandDraw();

                    if (!GetModel()->GetDrawableDynamicFlagVertexPositionsDidChange(clipDrawIndex))
                    {
                        continue;
                    }

                    IsCulling(GetModel()->GetDrawableCulling(clipDrawIndex) != 0);

                    {
                        csmFloat32* vertices = const_cast<csmFloat32*>(GetModel()->GetDrawableVertices(clipDrawIndex));
                        Core::csmVector2* uvs = const_cast<Core::csmVector2*>(GetModel()->GetDrawableVertexUvs(clipDrawIndex));
                        csmUint16* vertexIndices = const_cast<csmUint16*>(GetModel()->GetDrawableVertexIndices(clipDrawIndex));
                        const csmUint32 vertexCount = GetModel()->GetDrawableVertexCount(clipDrawIndex);
                        const csmUint32 vertexIndexCount = GetModel()->GetDrawableVertexIndexCount(clipDrawIndex);

                        CubismCommandBuffer_Metal::DrawCommandBuffer* drawCommandBufferMask = clipContext->_clippingCommandBufferList->At(index);
                        drawCommandBufferMask->UpdateVertexBuffer(vertices, uvs, vertexCount);
                        if (vertexIndexCount > 0)
                        {
                            drawCommandBufferMask->UpdateIndexBuffer(vertexIndices, vertexIndexCount);
                        }

                        if (vertexCount <= 0)
                        {
                            continue;
                        }
                    }

                    SetClippingContextBufferForMask(clipContext);

                    DrawMeshMetal(clipContext->_clippingCommandBufferList->At(index),
                                renderEncoder, *GetModel(), clipDrawIndex);
                }
            }

            {
                PostDraw(renderEncoder);
            }
        }

        CubismCommandBuffer_Metal::DrawCommandBuffer::DrawCommand* drawCommandDraw = _drawableDrawCommandBuffer[drawableIndex]->GetCommandDraw();
        _drawableDrawCommandBuffer[drawableIndex]->SetCommandBuffer(s_commandBuffer);

        SetClippingContextBufferForDraw(clipContext);

        IsCulling(GetModel()->GetDrawableCulling(drawableIndex) != 0);

        if (GetModel()->GetDrawableVertexIndexCount(drawableIndex) <= 0)
        {
            continue;
        }

        if(IsUsingHighPrecisionMask())
        {
            renderEncoder = PreDraw(s_commandBuffer, s_renderPassDescriptor);
        }

        DrawMeshMetal(_drawableDrawCommandBuffer[drawableIndex], renderEncoder, *GetModel(), drawableIndex);

        if(IsUsingHighPrecisionMask())
        {
            PostDraw(renderEncoder);
        }
    }

    if(!IsUsingHighPrecisionMask())
    {
        PostDraw(renderEncoder);
    }
}

void CubismRenderer_Metal::DrawMeshMetal(CubismCommandBuffer_Metal::DrawCommandBuffer* drawCommandBuffer
                                    , id <MTLRenderCommandEncoder> renderEncoder
                                    , const CubismModel& model, const csmInt32 index)
{
#ifndef CSM_DEBUG
    if (_textures[model.GetDrawableTextureIndex(index)] == 0) return;
#endif

    if (IsCulling())
    {
        [renderEncoder setCullMode:MTLCullModeBack];
    }
    else
    {
        [renderEncoder setCullMode:MTLCullModeNone];
    }

    [renderEncoder setFrontFacingWinding:MTLWindingCounterClockwise];

    if (IsGeneratingMask())
    {
        CubismShader_Metal::GetInstance()->SetupShaderProgramForMask(drawCommandBuffer, renderEncoder, this, model, index);
    }
    else
    {
        CubismShader_Metal::GetInstance()->SetupShaderProgramForDraw(drawCommandBuffer, renderEncoder, this, model, index);
    }

    id <MTLRenderPipelineState> pipelineState = drawCommandBuffer->GetCommandDraw()->GetRenderPipelineState();
    [renderEncoder setRenderPipelineState:pipelineState];

    {
        csmInt32 indexCount = model.GetDrawableVertexIndexCount(index);
        id <MTLBuffer> indexBuffer = drawCommandBuffer->GetIndexBuffer();
        [renderEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:indexCount indexType:MTLIndexTypeUInt16
                             indexBuffer:indexBuffer indexBufferOffset:0];
    }

    SetClippingContextBufferForDraw(NULL);
    SetClippingContextBufferForMask(NULL);
}

CubismCommandBuffer_Metal::DrawCommandBuffer* CubismRenderer_Metal::GetDrawCommandBufferData(csmInt32 drawableIndex)
{
    return _drawableDrawCommandBuffer[drawableIndex];
}

void CubismRenderer_Metal::SaveProfile()
{
}

void CubismRenderer_Metal::RestoreProfile()
{
}

void CubismRenderer_Metal::BindTexture(csmUint32 modelTextureIndex, id <MTLTexture> texture)
{
    _textures[modelTextureIndex] = texture;
}

const csmMap< csmInt32, id <MTLTexture> >& CubismRenderer_Metal::GetBindedTextures() const
{
    return _textures;
}

id<MTLTexture> CubismRenderer_Metal::GetBindedTextureId(csmInt32 textureId)
{
    return _textures[textureId];
}

void CubismRenderer_Metal::SetClippingMaskBufferSize(csmFloat32 width, csmFloat32 height)
{
    if (_clippingManager == NULL)
    {
        return;
    }

    const csmInt32 renderTextureCount = _clippingManager->GetRenderTextureCount();

    CSM_DELETE_SELF(CubismClippingManager_Metal, _clippingManager);

    _clippingManager = CSM_NEW CubismClippingManager_Metal();

    _clippingManager->SetClippingMaskBufferSize(width, height);

    _clippingManager->Initialize(
        *GetModel(),
        renderTextureCount
    );
}

csmInt32 CubismRenderer_Metal::GetRenderTextureCount() const
{
    return _clippingManager->GetRenderTextureCount();
}

CubismVector2 CubismRenderer_Metal::GetClippingMaskBufferSize() const
{
    return _clippingManager->GetClippingMaskBufferSize();
}

CubismOffscreenSurface_Metal* CubismRenderer_Metal::GetOffscreenSurface(csmInt32 index)
{
    return &_offscreenSurfaces[index];
}

void CubismRenderer_Metal::SetClippingContextBufferForMask(CubismClippingContext_Metal* clip)
{
    _clippingContextBufferForMask = clip;
}

CubismClippingContext_Metal* CubismRenderer_Metal::GetClippingContextBufferForMask() const
{
    return _clippingContextBufferForMask;
}

void CubismRenderer_Metal::SetClippingContextBufferForDraw(CubismClippingContext_Metal* clip)
{
    _clippingContextBufferForDraw = clip;
}

CubismClippingContext_Metal* CubismRenderer_Metal::GetClippingContextBufferForDraw() const
{
    return _clippingContextBufferForDraw;
}

const inline csmBool CubismRenderer_Metal::IsGeneratingMask() const
{
    return (GetClippingContextBufferForMask() != NULL);
}

}}}}

