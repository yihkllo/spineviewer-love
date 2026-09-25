

#pragma once

#include "CubismFramework.hpp"
#include "Type/csmVector.hpp"
#include "Type/csmRectF.hpp"
#include "Type/csmMap.hpp"
#include <float.h>

#ifdef CSM_TARGET_ANDROID_ES2
#include <jni.h>
#include <errno.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#ifdef CSM_TARGET_IPHONE_ES2
#include <OpenGLES/ES2/gl.h>
#include <OpenGLES/ES2/glext.h>
#endif

#if defined(CSM_TARGET_WIN_GL) || defined(CSM_TARGET_LINUX_GL)
#include <GL/glew.h>
#include <GL/gl.h>
#endif

#ifdef CSM_TARGET_MAC_GL
#ifndef CSM_TARGET_COCOS
#include <GL/glew.h>
#endif
#include <OpenGL/gl.h>
#endif

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {


class CubismOffscreenSurface_OpenGLES2
{
public:

    CubismOffscreenSurface_OpenGLES2();

    void BeginDraw(GLint restoreFBO = -1);

    void EndDraw();

    void Clear(float r, float g, float b, float a);

    csmBool CreateOffscreenSurface(csmUint32 displayBufferWidth, csmUint32 displayBufferHeight, GLuint colorBuffer = 0);

    void DestroyOffscreenSurface();

    GLuint GetRenderTexture() const;

    GLuint GetColorBuffer() const;

    csmUint32 GetBufferWidth() const;

    csmUint32 GetBufferHeight() const;

    csmBool IsValid() const;

private:
    GLuint      _renderTexture;
    GLuint      _colorBuffer;

    GLint       _oldFBO;

    csmUint32   _bufferWidth;
    csmUint32   _bufferHeight;
    csmBool     _isColorBufferInherited;
};


}}}}

