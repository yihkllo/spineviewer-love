

#pragma once

#include "CubismNativeInclude_D3D11.hpp"

#include "Math/CubismMatrix44.hpp"

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {


class CubismOffscreenSurface_D3D11
{
public:

    CubismOffscreenSurface_D3D11();

    void BeginDraw(ID3D11DeviceContext* renderContext);

    void EndDraw(ID3D11DeviceContext* renderContext);

    void Clear(ID3D11DeviceContext* renderContext, float r, float g, float b, float a);

    csmBool CreateOffscreenSurface(ID3D11Device* device, csmUint32 displayBufferWidth, csmUint32 displayBufferHeight);

    void DestroyOffscreenSurface();

    void SetClearColor(float r, float g, float b, float a);

    ID3D11ShaderResourceView* GetTextureView() const;

    csmUint32 GetBufferWidth() const;

    csmUint32 GetBufferHeight() const;

    csmBool IsValid() const;


private:
    ID3D11Texture2D*            _texture;
    ID3D11ShaderResourceView*   _textureView;
    ID3D11RenderTargetView*     _renderTargetView;
    ID3D11Texture2D*            _depthTexture;
    ID3D11DepthStencilView*     _depthView;

    ID3D11RenderTargetView*     _backupRender;
    ID3D11DepthStencilView*     _backupDepth;

    csmUint32                   _bufferWidth;
    csmUint32                   _bufferHeight;

};


}}}}

