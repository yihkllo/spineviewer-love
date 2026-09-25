

#include "CubismOffscreenSurface_OpenGLES2.hpp"

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

CubismOffscreenSurface_OpenGLES2::CubismOffscreenSurface_OpenGLES2()
    : _renderTexture(0)
    , _colorBuffer(0)
    , _oldFBO(0)
    , _bufferWidth(0)
    , _bufferHeight(0)
    , _isColorBufferInherited(false)
{
}


void CubismOffscreenSurface_OpenGLES2::BeginDraw(GLint restoreFBO)
{
    if (_renderTexture == 0)
    {
        return;
    }

    if (restoreFBO < 0)
    {
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &_oldFBO);
    }
    else
    {
        _oldFBO = restoreFBO;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, _renderTexture);
}

void CubismOffscreenSurface_OpenGLES2::EndDraw()
{
    if (_renderTexture == 0)
    {
        return;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, _oldFBO);
}

void CubismOffscreenSurface_OpenGLES2::Clear(float r, float g, float b, float a)
{
    glClearColor(r,g,b,a);
    glClear(GL_COLOR_BUFFER_BIT);
}

csmBool CubismOffscreenSurface_OpenGLES2::CreateOffscreenSurface(csmUint32 displayBufferWidth, csmUint32 displayBufferHeight, GLuint colorBuffer)
{
    DestroyOffscreenSurface();

    do
    {
        GLuint ret = 0;

        if (colorBuffer == 0)
        {
            glGenTextures(1, &_colorBuffer);

            glBindTexture(GL_TEXTURE_2D, _colorBuffer);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, displayBufferWidth, displayBufferHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glBindTexture(GL_TEXTURE_2D, 0);

            _isColorBufferInherited = false;
        }
        else
        {
            _colorBuffer = colorBuffer;

            _isColorBufferInherited = true;
        }

        GLint tmpFramebufferObject;
        glGetIntegerv(GL_FRAMEBUFFER_BINDING, &tmpFramebufferObject);

        glGenFramebuffers(1, &ret);
        glBindFramebuffer(GL_FRAMEBUFFER, ret);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _colorBuffer, 0);
        glBindFramebuffer(GL_FRAMEBUFFER, tmpFramebufferObject);

        _renderTexture = ret;

        _bufferWidth = displayBufferWidth;
        _bufferHeight = displayBufferHeight;

        return true;

    } while (0);

    DestroyOffscreenSurface();

    return false;
}

void CubismOffscreenSurface_OpenGLES2::DestroyOffscreenSurface()
{
    if (!_isColorBufferInherited && (_colorBuffer != 0))
    {
        glDeleteTextures(1, &_colorBuffer);
        _colorBuffer = 0;
    }

    if (_renderTexture!=0)
    {
        glDeleteFramebuffers(1, &_renderTexture);
        _renderTexture = 0;
    }
}

GLuint CubismOffscreenSurface_OpenGLES2::GetRenderTexture() const
{
    return _renderTexture;
}

GLuint CubismOffscreenSurface_OpenGLES2::GetColorBuffer() const
{
    return _colorBuffer;
}

csmUint32 CubismOffscreenSurface_OpenGLES2::GetBufferWidth() const
{
    return _bufferWidth;
}

csmUint32 CubismOffscreenSurface_OpenGLES2::GetBufferHeight() const
{
    return _bufferHeight;
}

csmBool CubismOffscreenSurface_OpenGLES2::IsValid() const
{
    return _renderTexture != 0;
}

}}}}

