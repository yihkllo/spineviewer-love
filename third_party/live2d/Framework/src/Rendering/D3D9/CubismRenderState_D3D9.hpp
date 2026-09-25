

#pragma once

#include "CubismNativeInclude_D3D9.hpp"

#include "Type/csmVector.hpp"
#include "Type/csmMap.hpp"

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

class CubismRenderer_D3D9;

class CubismRenderState_D3D9
{
    friend class CubismRenderer_D3D9;
public:
    enum
    {
        State_None,
        State_Blend,
        State_Viewport,
        State_ColorMask,
        State_RenderTarget,
        State_DepthTarget,
        State_ZEnable,
        State_CullMode,
        State_TextureFilterStage0,
        State_TextureFilterStage1,
        State_Max,
    };

    struct Stored
    {
        Stored()
        {
            BlendEnable = false;
            BlendAlphaSeparateEnable = false;
            BlendSourceMul = D3DBLEND_ZERO;
            BlendBlendFunc = D3DBLENDOP_ADD;
            BlendDestMul = D3DBLEND_ZERO;
            BlendSourceAlpha = D3DBLEND_ZERO;
            BlendAlphaFunc = D3DBLENDOP_ADD;
            BlendDestAlpha = D3DBLEND_ZERO;

            ViewportX = 0;
            ViewportY = 0;
            ViewportWidth = 0;
            ViewportHeight = 0;
            ViewportMinZ = 0.0f;
            ViewportMaxZ = 0.0f;

            ColorMaskMask = 0;

            ZEnable = D3DZB_FALSE;
            ZFunc = D3DCMP_NEVER;

            CullModeFaceMode = D3DCULL_NONE;

            MinFilter[0] = D3DTEXF_NONE;
            MagFilter[0] = D3DTEXF_NONE;
            MipFilter[0] = D3DTEXF_NONE;
            AddressU[0] = D3DTADDRESS_WRAP;
            AddressV[0] = D3DTADDRESS_WRAP;
            Anisotropy[0] = 0.0f;

            MinFilter[1] = D3DTEXF_NONE;
            MagFilter[1] = D3DTEXF_NONE;
            MipFilter[1] = D3DTEXF_NONE;
            AddressU[1] = D3DTADDRESS_WRAP;
            AddressV[1] = D3DTADDRESS_WRAP;
            Anisotropy[1] = 0.0f;

            memset(_valid, 0, sizeof(_valid));
        }

        bool BlendEnable;
        bool BlendAlphaSeparateEnable;
        D3DBLEND BlendSourceMul;
        D3DBLENDOP BlendBlendFunc;
        D3DBLEND BlendDestMul;
        D3DBLEND BlendSourceAlpha;
        D3DBLENDOP BlendAlphaFunc;
        D3DBLEND BlendDestAlpha;

        DWORD ViewportX;
        DWORD ViewportY;
        DWORD ViewportWidth;
        DWORD ViewportHeight;
        float ViewportMinZ;
        float ViewportMaxZ;

        DWORD ColorMaskMask;

        D3DZBUFFERTYPE ZEnable;
        D3DCMPFUNC ZFunc;

        D3DCULL CullModeFaceMode;

        D3DTEXTUREFILTERTYPE    MinFilter[2];
        D3DTEXTUREFILTERTYPE    MagFilter[2];
        D3DTEXTUREFILTERTYPE    MipFilter[2];
        D3DTEXTUREADDRESS       AddressU[2];
        D3DTEXTUREADDRESS       AddressV[2];
        float                   Anisotropy[2];

        csmBool _valid[State_Max];
    };

    void StartFrame();

    void Save();


    void Restore(LPDIRECT3DDEVICE9 device);


    void SetBlend(LPDIRECT3DDEVICE9 device, bool enable, bool alphaSeparateEnable,
        D3DBLEND srcmul, D3DBLENDOP blendFunc, D3DBLEND destmul,
        D3DBLEND srcalpha, D3DBLENDOP alphaFunc, D3DBLEND destalpha,
        csmBool force=false);

    void SetViewport(LPDIRECT3DDEVICE9 device, DWORD left, DWORD top, DWORD width, DWORD height, float zMin, float zMax, csmBool force = false);

    void SetColorMask(LPDIRECT3DDEVICE9 device, DWORD mask, csmBool force = false);

    void SetZEnable(LPDIRECT3DDEVICE9 device, D3DZBUFFERTYPE enable, D3DCMPFUNC zfunc, csmBool force = false);

    void SetCullMode(LPDIRECT3DDEVICE9 device, D3DCULL cullFace, csmBool force = false);

    void SetTextureFilter(LPDIRECT3DDEVICE9 device, csmInt32 stage, D3DTEXTUREFILTERTYPE minFilter, D3DTEXTUREFILTERTYPE magFilter, D3DTEXTUREFILTERTYPE mipFilter, D3DTEXTUREADDRESS addressU, D3DTEXTUREADDRESS addressV, csmFloat32 anisotropy = 0.0, csmBool force = false);

private:
    CubismRenderState_D3D9();
    ~CubismRenderState_D3D9();

    void SaveCurrentNativeState(LPDIRECT3DDEVICE9 device);

    void RestoreNativeState(LPDIRECT3DDEVICE9 device);

    Stored  _stored;

    csmVector<Stored> _pushed;
};

}}}}
