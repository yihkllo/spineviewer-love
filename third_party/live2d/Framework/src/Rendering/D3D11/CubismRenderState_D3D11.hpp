

#pragma once

#include "CubismNativeInclude_D3D11.hpp"

#include "Type/csmVector.hpp"
#include "Type/csmMap.hpp"

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

class CubismRenderer_D3D11;

class CubismRenderState_D3D11
{
    friend class CubismRenderer_D3D11;
public:

    enum
    {
        State_None,
        State_Blend,
        State_Viewport,
        State_ZEnable,
        State_CullMode,
        State_Sampler,
        State_Max,
    };

    enum Blend
    {
        Blend_Origin,
        Blend_Zero,
        Blend_Normal,
        Blend_Add,
        Blend_Mult,
        Blend_Mask,
        Blend_Max,
    };

    enum Cull
    {
        Cull_Origin,
        Cull_None,
        Cull_Ccw,
        Cull_Max,
    };

    enum Depth
    {
        Depth_Origin,
        Depth_Disable,
        Depth_Enable,
        Depth_Max,
    };

    enum Sampler
    {
        Sampler_Origin,
        Sampler_Normal,
        Sampler_Anisotropy,
        Sampler_Max,
    };

    struct Stored
    {
        Stored()
        {
            _blendState = Blend_Zero;
            _blendFactor.x = _blendFactor.y = _blendFactor.z = _blendFactor.w = 0.0f;
            _blendMask = 0xffffffff;

            _cullMode = Cull_None;

            _depthEnable = Depth_Disable;
            _depthRef = 0;

            _viewportX = 0;
            _viewportY = 0;
            _viewportWidth = 0;
            _viewportHeight = 0;
            _viewportMinZ = 0.0f;
            _viewportMaxZ = 0.0f;

            _sampler = Sampler_Normal;
            _anisotropy = 0.0;

            memset(_valid, 0, sizeof(_valid));
        }

        Blend _blendState;
        DirectX::XMFLOAT4 _blendFactor;
        UINT _blendMask;

        Cull _cullMode;

        FLOAT _viewportX;
        FLOAT _viewportY;
        FLOAT _viewportWidth;
        FLOAT _viewportHeight;
        FLOAT _viewportMinZ;
        FLOAT _viewportMaxZ;

        Depth _depthEnable;
        UINT _depthRef;

        Sampler _sampler;
        FLOAT _anisotropy;

        csmBool _valid[State_Max];
    };

    void StartFrame();

    void Save();


    void Restore(ID3D11DeviceContext* renderContext);

    void SetBlend(ID3D11DeviceContext* renderContext, Blend blendState, DirectX::XMFLOAT4 blendFactor, UINT mask,
        csmBool force=false);

    void SetCullMode(ID3D11DeviceContext* renderContext, Cull cullFace, csmBool force = false);

    void SetViewport(ID3D11DeviceContext* renderContext, FLOAT left, FLOAT top, FLOAT width, FLOAT height, FLOAT zMin, FLOAT zMax, csmBool force = false);

    void SetZEnable(ID3D11DeviceContext* renderContext, Depth enable, UINT stelcilRef, csmBool force = false);

    void SetSampler(ID3D11DeviceContext* renderContext, Sampler sample, csmFloat32 anisotropy = 0.0, csmBool force = false);

private:
    CubismRenderState_D3D11();
    ~CubismRenderState_D3D11();

    void Create(ID3D11Device* device);

    void SaveCurrentNativeState(ID3D11Device* device, ID3D11DeviceContext* renderContext);

    void RestoreNativeState(ID3D11Device* device, ID3D11DeviceContext* renderContext);

    Stored  _stored;

    csmVector<Stored> _pushed;


    csmVector<ID3D11BlendState*>        _blendStateObjects;
    csmVector<ID3D11RasterizerState*>   _rasterizeStateObjects;
    csmVector<ID3D11DepthStencilState*> _depthStencilState;
    csmVector<ID3D11SamplerState*>      _samplerState;
};

}}}}
