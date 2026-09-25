

#include "CubismOffscreenSurface_D3D9.hpp"

#include "CubismRenderer_D3D9.hpp"
#include "CubismShader_D3D9.hpp"

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

CubismOffscreenSurface_D3D9::CubismOffscreenSurface_D3D9()
    : _texture(NULL)
    , _textureSurface(NULL)
    , _depthSurface(NULL)
    , _backupRender(NULL)
    , _backupDepth(NULL)
    , _bufferWidth(0)
    , _bufferHeight(0)
{
}


void CubismOffscreenSurface_D3D9::BeginDraw(LPDIRECT3DDEVICE9 device)
{
    if(_depthSurface==NULL || _texture==NULL)
    {
        return;
    }

    device->EndScene();

    device->BeginScene();

    _backupRender = NULL;
    _backupDepth = NULL;

    device->GetRenderTarget(0, &_backupRender);
    device->GetDepthStencilSurface(&_backupDepth);


    LPDIRECT3DSURFACE9 surface;
    _textureSurface = NULL;
    if (SUCCEEDED(_texture->GetSurfaceLevel(0, &surface)))
    {
        _textureSurface = surface;

        device->SetRenderTarget(0, surface);
        device->SetDepthStencilSurface(_depthSurface);
    }
}

void CubismOffscreenSurface_D3D9::EndDraw(LPDIRECT3DDEVICE9 device)
{
    if (_depthSurface == NULL || _texture == NULL)
    {
        return;
    }

    device->EndScene();

    if (_textureSurface)
    {
        device->SetRenderTarget(0, _backupRender);
        device->SetDepthStencilSurface(_backupDepth);
        {
            _textureSurface->Release();
            _textureSurface = NULL;
        }
    }

    if(_backupDepth)
    {
        _backupDepth->Release();
        _backupDepth = NULL;
    }
    if(_backupRender)
    {
        _backupRender->Release();
        _backupRender = NULL;
    }

    device->BeginScene();
}

void CubismOffscreenSurface_D3D9::Clear(LPDIRECT3DDEVICE9 device,  float r, float g, float b, float a)
{
    device->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        D3DCOLOR_COLORVALUE(r, g, b, a), 1.0f, 0);
}

csmBool CubismOffscreenSurface_D3D9::CreateOffscreenSurface(LPDIRECT3DDEVICE9 device, csmUint32 displayBufferWidth, csmUint32 displayBufferHeight)
{
    DestroyOffscreenSurface();

    if (FAILED(D3DXCreateTexture(
        device,
        displayBufferWidth,
        displayBufferHeight,
        0,
        D3DUSAGE_RENDERTARGET,
        D3DFMT_A8R8G8B8,
        D3DPOOL_DEFAULT,
        &_texture)))
    {
        return false;
    }

    if (FAILED(device->CreateDepthStencilSurface(
        displayBufferWidth,
        displayBufferHeight,
        D3DFMT_D16,
        D3DMULTISAMPLE_NONE,
        0,
        TRUE,
        &_depthSurface,
        NULL)))
    {
        if(_texture)
        {
            _texture->Release();
            _texture = NULL;
        }
        return false;
    }

    _bufferWidth = displayBufferWidth;
    _bufferHeight = displayBufferHeight;

    return true;
}

void CubismOffscreenSurface_D3D9::DestroyOffscreenSurface()
{
    if(_backupDepth)
    {
        _backupDepth->Release();
        _backupDepth = NULL;
    }
    if (_backupRender)
    {
        _backupRender->Release();
        _backupRender = NULL;
    }
    if (_textureSurface)
    {
        _textureSurface->Release();
        _textureSurface = NULL;
    }


    if(_depthSurface)
    {
        _depthSurface->Release();
        _depthSurface = NULL;
    }
    if (_texture)
    {
        _texture->Release();
        _texture = NULL;
    }
}

LPDIRECT3DTEXTURE9 CubismOffscreenSurface_D3D9::GetTexture() const
{
    return _texture;
}

csmUint32 CubismOffscreenSurface_D3D9::GetBufferWidth() const
{
    return _bufferWidth;
}

csmUint32 CubismOffscreenSurface_D3D9::GetBufferHeight() const
{
    return _bufferHeight;
}

csmBool CubismOffscreenSurface_D3D9::IsValid() const
{
    if (_depthSurface == NULL || _texture == NULL)
    {
        return false;
    }

    return true;
}

}}}}

