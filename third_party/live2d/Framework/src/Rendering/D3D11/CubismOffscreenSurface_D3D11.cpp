

#include "CubismOffscreenSurface_D3D11.hpp"

#include "CubismRenderer_D3D11.hpp"

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

CubismOffscreenSurface_D3D11::CubismOffscreenSurface_D3D11()
    : _texture(NULL)
    , _textureView(NULL)
    , _renderTargetView(NULL)
    , _depthTexture(NULL)
    , _depthView(NULL)
    , _backupRender(NULL)
    , _backupDepth(NULL)
    , _bufferWidth(0)
    , _bufferHeight(0)
{
}


void CubismOffscreenSurface_D3D11::BeginDraw(ID3D11DeviceContext* renderContext)
{
    if(_textureView==NULL || _renderTargetView==NULL || _depthView==NULL)
    {
        return;
    }

    _backupRender = NULL;
    _backupDepth = NULL;

    renderContext->OMGetRenderTargets(1, &_backupRender, &_backupDepth);

    {
        ID3D11ShaderResourceView* const viewArray[2] = { NULL, NULL };
        renderContext->PSSetShaderResources(0, 2, viewArray);
    }

    renderContext->OMSetRenderTargets( 1, &_renderTargetView, _depthView);
}

void CubismOffscreenSurface_D3D11::EndDraw(ID3D11DeviceContext* renderContext)
{
    if (_textureView == NULL || _renderTargetView == NULL || _depthView == NULL)
    {
        return;
    }

    renderContext->OMSetRenderTargets(1, &_backupRender, _backupDepth);

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
}

void CubismOffscreenSurface_D3D11::Clear(ID3D11DeviceContext* renderContext, float r, float g, float b, float a)
{
    float clearColor[4] = {r,g,b,a};
    renderContext->ClearRenderTargetView(_renderTargetView, clearColor);
    renderContext->ClearDepthStencilView(_depthView, D3D11_CLEAR_DEPTH, 1.0f, 0);
}

csmBool CubismOffscreenSurface_D3D11::CreateOffscreenSurface(ID3D11Device* device, csmUint32 displayBufferWidth, csmUint32 displayBufferHeight)
{
    DestroyOffscreenSurface();

    do
    {
        HRESULT result;

        D3D11_TEXTURE2D_DESC textureDesc;
        memset(&textureDesc, 0, sizeof(textureDesc));
        textureDesc.Usage = D3D11_USAGE_DEFAULT;
        textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
        textureDesc.Width = displayBufferWidth;
        textureDesc.Height = displayBufferHeight;
        textureDesc.CPUAccessFlags = 0;
        textureDesc.MipLevels = 1;
        textureDesc.ArraySize = 1;
        textureDesc.SampleDesc.Count = 1;
        textureDesc.SampleDesc.Quality = 0;
        result = device->CreateTexture2D(&textureDesc, NULL, &_texture);
        if (FAILED(result))
        {
            CubismLogError("Error : create offscreen texture");
            break;
        }

        D3D11_RENDER_TARGET_VIEW_DESC renderTargetDesc;
        memset( &renderTargetDesc, 0, sizeof(renderTargetDesc) );
        renderTargetDesc.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
        renderTargetDesc.ViewDimension      = D3D11_RTV_DIMENSION_TEXTURE2D;
        result = device->CreateRenderTargetView( _texture, &renderTargetDesc, &_renderTargetView );
        if ( FAILED(result) )
        {
            CubismLogError("Error : create offscreen rendertarget view");
            break;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC resourceViewDesc;
        memset(&resourceViewDesc, 0, sizeof(resourceViewDesc));
        resourceViewDesc.Format = renderTargetDesc.Format;
        resourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        resourceViewDesc.Texture2D.MipLevels = 1;
        result = device->CreateShaderResourceView(_texture, &resourceViewDesc, &_textureView);
        if (FAILED(result))
        {
            CubismLogError("Error : create offscreen resource view");
            break;
        }

        D3D11_TEXTURE2D_DESC depthDesc;
        memset(&depthDesc, 0, sizeof(depthDesc));
        depthDesc.Width = displayBufferWidth;
        depthDesc.Height = displayBufferHeight;
        depthDesc.MipLevels = 1;
        depthDesc.ArraySize = 1;
        depthDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
        depthDesc.SampleDesc.Count = 1;
        depthDesc.SampleDesc.Quality = 0;
        depthDesc.Usage = D3D11_USAGE_DEFAULT;
        depthDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
        depthDesc.CPUAccessFlags = 0;
        depthDesc.MiscFlags = 0;
        result = device->CreateTexture2D(&depthDesc, NULL, &_depthTexture);
        if (FAILED(result))
        {
            CubismLogError("Error : create offscreen depth texture");
            break;
        }

        D3D11_DEPTH_STENCIL_VIEW_DESC depthViewDesc;
        memset(&depthViewDesc, 0, sizeof(depthViewDesc));
        depthViewDesc.Format = depthDesc.Format;
        depthViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
        depthViewDesc.Texture2D.MipSlice = 0;
        result = device->CreateDepthStencilView(_depthTexture, &depthViewDesc, &_depthView);
        if (FAILED(result))
        {
            CubismLogError("Error : create offscreen depth view");
            break;
        }

        _bufferWidth = displayBufferWidth;
        _bufferHeight = displayBufferHeight;

        return true;

    } while (0);

    DestroyOffscreenSurface();

    return false;
}

void CubismOffscreenSurface_D3D11::DestroyOffscreenSurface()
{
    if (_backupDepth)
    {
        _backupDepth->Release();
        _backupDepth = NULL;
    }
    if (_backupRender)
    {
        _backupRender->Release();
        _backupRender = NULL;
    }


    if (_depthView)
    {
        _depthView->Release();
        _depthView = NULL;
    }
    if (_depthTexture)
    {
        _depthTexture->Release();
        _depthTexture = NULL;
    }
    if (_textureView)
    {
        _textureView->Release();
        _textureView = NULL;
    }
    if (_renderTargetView)
    {
        _renderTargetView->Release();
        _renderTargetView = NULL;
    }
    if (_texture)
    {
        _texture->Release();
        _texture = NULL;
    }
}

ID3D11ShaderResourceView* CubismOffscreenSurface_D3D11::GetTextureView() const
{
    return _textureView;
}

csmUint32 CubismOffscreenSurface_D3D11::GetBufferWidth() const
{
    return _bufferWidth;
}

csmUint32 CubismOffscreenSurface_D3D11::GetBufferHeight() const
{
    return _bufferHeight;
}

csmBool CubismOffscreenSurface_D3D11::IsValid() const
{
    if (_textureView == NULL || _renderTargetView == NULL || _depthView == NULL)
    {
        return false;
    }

    return true;
}

}}}}

