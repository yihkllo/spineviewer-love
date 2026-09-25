

#pragma once

#include "CubismNativeInclude_D3D9.hpp"

#include "Math/CubismMatrix44.hpp"

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {


class CubismOffscreenSurface_D3D9
{
public:

    CubismOffscreenSurface_D3D9();

    void BeginDraw(LPDIRECT3DDEVICE9 device);

    void EndDraw(LPDIRECT3DDEVICE9 device);

    void Clear(LPDIRECT3DDEVICE9 device, float r, float g, float b, float a);

    csmBool CreateOffscreenSurface(LPDIRECT3DDEVICE9 device, csmUint32 displayBufferWidth, csmUint32 displayBufferHeight);

    void DestroyOffscreenSurface();

    LPDIRECT3DTEXTURE9 GetTexture() const;

    csmUint32 GetBufferWidth() const;

    csmUint32 GetBufferHeight() const;

    csmBool IsValid() const;

private:
    LPDIRECT3DTEXTURE9  _texture;
    LPDIRECT3DSURFACE9  _textureSurface;
    LPDIRECT3DSURFACE9  _depthSurface;

    LPDIRECT3DSURFACE9  _backupRender;
    LPDIRECT3DSURFACE9  _backupDepth;

    csmUint32           _bufferWidth;
    csmUint32           _bufferHeight;
};


}}}}

