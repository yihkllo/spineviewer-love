

#include "CubismRenderState_D3D11.hpp"
#include "CubismShader_D3D11.hpp"
#include "CubismRenderer_D3D11.hpp"
#include "Type/csmVector.hpp"

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

CubismRenderState_D3D11::CubismRenderState_D3D11()
{
    memset(_stored._valid, 0, sizeof(_stored._valid));

    Create(CubismRenderer_D3D11::GetCurrentDevice());
}

CubismRenderState_D3D11::~CubismRenderState_D3D11()
{
    _pushed.Clear();

    for (csmUint32 i = 0; i<_samplerState.GetSize(); i++)
    {
        ID3D11SamplerState* sampl = _samplerState[i];
        if (sampl)
        {
            sampl->Release();
        }
    }
    _samplerState.Clear();

    for (csmUint32 i = 0; i<_depthStencilState.GetSize(); i++)
    {
        ID3D11DepthStencilState* depth = _depthStencilState[i];
        if (depth)
        {
            depth->Release();
        }
    }
    _depthStencilState.Clear();

    for (csmUint32 i = 0; i<_rasterizeStateObjects.GetSize(); i++)
    {
        ID3D11RasterizerState* raster = _rasterizeStateObjects[i];
        if (raster)
        {
            raster->Release();
        }
    }
    _rasterizeStateObjects.Clear();

    for( csmUint32 i=0; i<_blendStateObjects.GetSize(); i++ )
    {
        ID3D11BlendState* state = _blendStateObjects[i];
        if(state)
        {
            state->Release();
        }
    }
    _blendStateObjects.Clear();
}

void CubismRenderState_D3D11::Create(ID3D11Device* device)
{
    if(!device)
    {
        return;
    }

    ID3D11BlendState* state = NULL;
    D3D11_BLEND_DESC blendDesc;
    memset(&blendDesc, 0, sizeof(blendDesc));

    _blendStateObjects.PushBack(NULL);

    blendDesc.AlphaToCoverageEnable = FALSE;
    blendDesc.IndependentBlendEnable = FALSE;
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    device->CreateBlendState(&blendDesc, &state);
    _blendStateObjects.PushBack(state);

    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    device->CreateBlendState(&blendDesc, &state);
    _blendStateObjects.PushBack(state);

    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    device->CreateBlendState(&blendDesc, &state);
    _blendStateObjects.PushBack(state);

    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_DEST_COLOR;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
    device->CreateBlendState(&blendDesc, &state);
    _blendStateObjects.PushBack(state);

    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_COLOR;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
    device->CreateBlendState(&blendDesc, &state);
    _blendStateObjects.PushBack(state);


    ID3D11RasterizerState* rasterizer = NULL;
    D3D11_RASTERIZER_DESC rasterDesc;
    memset(&rasterDesc, 0, sizeof(rasterDesc));

    _rasterizeStateObjects.PushBack(NULL);

    rasterDesc.FillMode = D3D11_FILL_MODE::D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_NONE;
    rasterDesc.FrontCounterClockwise = TRUE;
    rasterDesc.DepthClipEnable = FALSE;
    rasterDesc.MultisampleEnable = FALSE;
    rasterDesc.DepthBiasClamp = 0;
    rasterDesc.SlopeScaledDepthBias = 0;
    device->CreateRasterizerState(&rasterDesc, &rasterizer);
    _rasterizeStateObjects.PushBack(rasterizer);

    rasterDesc.CullMode = D3D11_CULL_MODE::D3D11_CULL_BACK;
    device->CreateRasterizerState(&rasterDesc, &rasterizer);
    _rasterizeStateObjects.PushBack(rasterizer);


    ID3D11DepthStencilState* depth = NULL;
    D3D11_DEPTH_STENCIL_DESC depthDesc;
    memset(&depthDesc, 0, sizeof(depthDesc));

    _depthStencilState.PushBack(NULL);

    depthDesc.DepthEnable = false;
    depthDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
    depthDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthDesc.StencilEnable = false;
    device->CreateDepthStencilState(&depthDesc, &depth);
    _depthStencilState.PushBack(depth);

    depthDesc.DepthEnable = true;
    depthDesc.StencilEnable = false;
    device->CreateDepthStencilState(&depthDesc, &depth);
    _depthStencilState.PushBack(depth);


    ID3D11SamplerState* sampler = NULL;
    D3D11_SAMPLER_DESC samplerDesc;
    memset(&samplerDesc, 0, sizeof(D3D11_SAMPLER_DESC));

    _samplerState.PushBack(NULL);

    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    samplerDesc.MaxAnisotropy = 0;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = -D3D11_FLOAT32_MAX;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    samplerDesc.BorderColor[0] = samplerDesc.BorderColor[1] = samplerDesc.BorderColor[2] = samplerDesc.BorderColor[3] = 1.0f;
    device->CreateSamplerState(&samplerDesc, &sampler);
    _samplerState.PushBack(sampler);

    samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
    samplerDesc.MaxAnisotropy = 2;
    device->CreateSamplerState(&samplerDesc, &sampler);
    _samplerState.PushBack(sampler);
}

void CubismRenderState_D3D11::StartFrame()
{
    memset(_stored._valid, 0, sizeof(_stored._valid));

    _pushed.Clear();
}

void CubismRenderState_D3D11::Save()
{
    _pushed.PushBack(_stored);
}

void CubismRenderState_D3D11::Restore(ID3D11DeviceContext* renderContext)
{
    const csmUint32 size = _pushed.GetSize();

    if (size == 0)
    {
        return;
    }

    csmBool isSet[State_Max];
    memset(isSet, 0, sizeof(isSet));

    for (csmInt32 i = static_cast<csmInt32>(_pushed.GetSize())-1; i>=0; i--)
    {
        Stored &current = _pushed[i];

        if (_pushed[i]._valid[State_Blend] && !isSet[State_Blend])
        {
            SetBlend(renderContext, current._blendState, current._blendFactor, current._blendMask, true);
            isSet[State_Blend] = true;
        }
        if (_pushed[i]._valid[State_CullMode] && !isSet[State_CullMode])
        {
            SetCullMode(renderContext, current._cullMode, true);
            isSet[State_CullMode] = true;
        }
        if (_pushed[i]._valid[State_Viewport] && !isSet[State_Viewport])
        {
            SetViewport(renderContext, current._viewportX, current._viewportY, current._viewportWidth, current._viewportHeight, current._viewportMinZ, current._viewportMaxZ, true);
            isSet[State_Viewport] = true;
        }
        if (_pushed[i]._valid[State_ZEnable] && !isSet[State_ZEnable])
        {
            SetZEnable(renderContext, current._depthEnable, current._depthRef, true);
            isSet[State_ZEnable] = true;
        }
        if (_pushed[i]._valid[State_Sampler] && !isSet[State_Sampler])
        {
            SetSampler(renderContext, current._sampler, true);
            isSet[State_Sampler] = true;
        }
    }

    Stored store = _pushed[size - 1];
    _pushed.Remove(size - 1);
    if(_pushed.GetSize()==0)
    {
        _pushed.Clear();
    }
    _stored = store;
}

void CubismRenderState_D3D11::SetBlend(ID3D11DeviceContext* renderContext, Blend blendState, DirectX::XMFLOAT4 blendFactor, UINT mask,
    csmBool force)
{
    if(!renderContext || blendState<0 || Blend_Max<= blendState)
    {
        return;
    }

    if (!_stored._valid[State_Blend] || force ||
        _stored._blendFactor.x != blendFactor.x || _stored._blendFactor.y != blendFactor.y || _stored._blendFactor.z != blendFactor.z || _stored._blendFactor.w != blendFactor.w ||
        _stored._blendMask != mask ||
        _stored._blendState != blendState)
    {
        const float factor[4] = { blendFactor.x, blendFactor.y, blendFactor.z, blendFactor.w };
        renderContext->OMSetBlendState(_blendStateObjects[blendState], factor, mask);
    }

    _stored._blendState = blendState;
    _stored._blendFactor = blendFactor;
    _stored._blendMask = mask;

    _stored._valid[State_Blend] = true;
}

void CubismRenderState_D3D11::SetCullMode(ID3D11DeviceContext* renderContext, Cull cullFace, csmBool force)
{
    if (!renderContext || cullFace<0 || Cull_Max <= cullFace)
    {
        return;
    }

    if (!_stored._valid[State_CullMode] || force ||
        _stored._cullMode != cullFace)
    {
        renderContext->RSSetState(_rasterizeStateObjects[cullFace]);
    }

    _stored._cullMode = cullFace;

    _stored._valid[State_CullMode] = true;
}

void CubismRenderState_D3D11::SetViewport(ID3D11DeviceContext* renderContext, FLOAT left, FLOAT top, FLOAT width, FLOAT height, FLOAT zMin, FLOAT zMax, csmBool force)
{
    if (!_stored._valid[State_Blend] || force ||
        _stored._viewportX != left || _stored._viewportY != top || _stored._viewportWidth != width || _stored._viewportHeight != height ||
        _stored._viewportMinZ != zMin || _stored._viewportMaxZ != zMax)
    {
        D3D11_VIEWPORT setViewport;
        setViewport.TopLeftX = left;
        setViewport.TopLeftY = top;
        setViewport.Width = width;
        setViewport.Height = height;
        setViewport.MinDepth = zMin;
        setViewport.MaxDepth = zMax;
        renderContext->RSSetViewports(1, &setViewport);
    }

    _stored._viewportX = left;
    _stored._viewportY = top;
    _stored._viewportWidth = width;
    _stored._viewportHeight = height;
    _stored._viewportMinZ = zMin;
    _stored._viewportMaxZ = zMax;

    _stored._valid[State_Viewport] = true;
}

void CubismRenderState_D3D11::SetZEnable(ID3D11DeviceContext* renderContext, Depth enable, UINT stelcilRef, csmBool force)
{
    if (!renderContext || enable<0 || Depth_Max <= enable)
    {
        return;
    }

    if (!_stored._valid[State_ZEnable] || force ||
        _stored._depthEnable != enable)
    {
        renderContext->OMSetDepthStencilState(_depthStencilState[enable], stelcilRef);
    }

    _stored._depthEnable = enable;
    _stored._depthRef = stelcilRef;

    _stored._valid[State_ZEnable] = true;
}

void CubismRenderState_D3D11::SetSampler(ID3D11DeviceContext* renderContext, Sampler sample, csmFloat32 anisotropy, csmBool force)
{
    if (!renderContext || sample<0 || Sampler_Max <= sample)
    {
        return;
    }

    if (!_stored._valid[State_ZEnable] || force ||
        _stored._sampler != sample)
    {
        if (anisotropy > 0.0 && sample == Sampler_Anisotropy) {
            ID3D11SamplerState* sampler;
            D3D11_SAMPLER_DESC samplerDesc;
            memset(&samplerDesc, 0, sizeof(D3D11_SAMPLER_DESC));
            renderContext->PSGetSamplers(0, 1, &sampler);
            sampler->GetDesc(&samplerDesc);
            samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
            samplerDesc.MaxAnisotropy = anisotropy;
        }

        renderContext->PSSetSamplers(0, 1, &_samplerState[sample]);
    }

    _stored._sampler = sample;

    _stored._valid[State_Sampler] = true;
}

void CubismRenderState_D3D11::SaveCurrentNativeState(ID3D11Device* device, ID3D11DeviceContext* renderContext)
{
    _pushed.Clear();
    memset(_stored._valid, 0, sizeof(_stored._valid));

    ID3D11BlendState* originBlend;
    FLOAT originFactor[4];
    UINT originMask;
    renderContext->OMGetBlendState(&originBlend, originFactor, &originMask);
    _blendStateObjects[Blend_Origin] = originBlend;
    SetBlend(renderContext, Blend_Origin, DirectX::XMFLOAT4(originFactor), originMask, true);

    ID3D11RasterizerState* originRaster;
    renderContext->RSGetState(&originRaster);
    _rasterizeStateObjects[Cull_Origin] = originRaster;
    SetCullMode(renderContext, Cull_Origin, true);

    ID3D11DepthStencilState* originDepth;
    UINT stelcilRef;
    renderContext->OMGetDepthStencilState(&originDepth, &stelcilRef);
    _depthStencilState[Depth_Origin] = originDepth;
    SetZEnable(renderContext, Depth_Origin, stelcilRef, true);

    ID3D11SamplerState* originSampler;
    renderContext->PSGetSamplers(0, 1, &originSampler);
    _samplerState[Sampler_Origin] = originSampler;
    SetSampler(renderContext, Sampler_Origin, true);

    UINT numViewport = 1;
    D3D11_VIEWPORT viewPort;
    renderContext->RSGetViewports(&numViewport, &viewPort);
    SetViewport(renderContext, viewPort.TopLeftX, viewPort.TopLeftY, viewPort.Width, viewPort.Height, viewPort.MinDepth, viewPort.MaxDepth, true);

    Save();
}

void CubismRenderState_D3D11::RestoreNativeState(ID3D11Device* device, ID3D11DeviceContext* renderContext)
{
    for (csmInt32 i = static_cast<csmInt32>(_pushed.GetSize()) - 1; i >= 0; i--)
    {
        Restore(renderContext);
    }

    if (_samplerState[Sampler_Origin])
    {
        _samplerState[Sampler_Origin]->Release();
        _samplerState[Sampler_Origin] = NULL;
    }
    if(_depthStencilState[Depth_Origin])
    {
        _depthStencilState[Depth_Origin]->Release();
        _depthStencilState[Depth_Origin] = NULL;
    }
    if (_rasterizeStateObjects[Cull_Origin])
    {
        _rasterizeStateObjects[Cull_Origin]->Release();
        _rasterizeStateObjects[Cull_Origin] = NULL;
    }
    if (_blendStateObjects[Blend_Origin])
    {
        _blendStateObjects[Blend_Origin]->Release();
        _blendStateObjects[Blend_Origin] = NULL;
    }
}

}}}}

