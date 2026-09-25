

#include "CubismOffscreenSurface_Cocos2dx.hpp"

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

CubismOffscreenSurface_Cocos2dx::CubismOffscreenSurface_Cocos2dx()
    : _renderTexture(NULL)
    , _colorBuffer(NULL)
    , _isInheritedRenderTexture(false)
    , _previousColorBuffer(NULL)
    , _bufferWidth(0)
    , _bufferHeight(0)
{
}


void CubismOffscreenSurface_Cocos2dx::BeginDraw(CubismCommandBuffer_Cocos2dx* commandBuffer, cocos2d::Texture2D* colorBufferOnFinishDrawing)
{
    if (!IsValid())
    {
        return;
    }

    if (colorBufferOnFinishDrawing == NULL)
    {
        _previousColorBuffer = commandBuffer->GetColorBuffer();
        if (_previousColorBuffer == NULL)
        {
            _previousColorBuffer = GetCocos2dRenderer()->getColorAttachment();
        }
    }
    else
    {
        _previousColorBuffer = colorBufferOnFinishDrawing;
    }

    commandBuffer->SetColorBuffer(_renderTexture->getSprite()->getTexture());
}

void CubismOffscreenSurface_Cocos2dx::EndDraw(CubismCommandBuffer_Cocos2dx* commandBuffer)
{
    if (!IsValid())
    {
        return;
    }

    commandBuffer->SetColorBuffer(_previousColorBuffer);
}

void CubismOffscreenSurface_Cocos2dx::Clear(CubismCommandBuffer_Cocos2dx* commandBuffer, float r, float g, float b, float a)
{
    commandBuffer->Clear(r, g, b, a);
}

csmBool CubismOffscreenSurface_Cocos2dx::CreateOffscreenSurface(csmUint32 displayBufferWidth, csmUint32 displayBufferHeight, cocos2d::RenderTexture* renderTexture)
{
    DestroyOffscreenSurface();

    do
    {
        if (!renderTexture)
        {


            csmBool initResult = false;


            _renderTexture = cocos2d::RenderTexture::create(displayBufferWidth, displayBufferHeight);

            if (!_renderTexture)
            {
                break;
            }

            _renderTexture->retain();


            _renderTexture->getSprite()->getTexture()->setTexParameters(
                cocos2d::Texture2D::TexParams(
                    cocos2d::backend::SamplerFilter::LINEAR,
                    cocos2d::backend::SamplerFilter::LINEAR,
                    cocos2d::backend::SamplerAddressMode::CLAMP_TO_EDGE,
                    cocos2d::backend::SamplerAddressMode::CLAMP_TO_EDGE
                )
            );

            _colorBuffer = _renderTexture->getSprite()->getTexture();
            _isInheritedRenderTexture = false;
        }
        else
        {
            _renderTexture = renderTexture;
            _colorBuffer = _renderTexture->getSprite()->getTexture();


            _isInheritedRenderTexture = true;
        }

        if (_colorBuffer)
        {
            _viewPortSize = csmRectF(0.0f, 0.0f, _colorBuffer->getContentSizeInPixels().width, _colorBuffer->getContentSizeInPixels().height);
        }
        else
        {
            _viewPortSize = csmRectF(0.0f, 0.0f, _bufferWidth, _bufferHeight);
        }

        _bufferWidth = displayBufferWidth;
        _bufferHeight = displayBufferHeight;


        return true;

    } while (0);

    DestroyOffscreenSurface();

    return false;
}

void CubismOffscreenSurface_Cocos2dx::DestroyOffscreenSurface()
{
    if ((_renderTexture != NULL) && !_isInheritedRenderTexture)
    {
        CC_SAFE_RELEASE_NULL(_renderTexture);
        _colorBuffer = NULL;
    }
}

cocos2d::Texture2D* CubismOffscreenSurface_Cocos2dx::GetColorBuffer() const
{
    return _renderTexture->getSprite()->getTexture();
}

csmUint32 CubismOffscreenSurface_Cocos2dx::GetBufferWidth() const
{
    return _bufferWidth;
}

csmUint32 CubismOffscreenSurface_Cocos2dx::GetBufferHeight() const
{
    return _bufferHeight;
}

csmRectF CubismOffscreenSurface_Cocos2dx::GetViewPortSize() const
{
    return _viewPortSize;
}

csmBool CubismOffscreenSurface_Cocos2dx::IsValid() const
{
    return _renderTexture != NULL;
}

}}}}

