

#pragma once

#include "CubismFramework.hpp"
#include "CubismCommandBuffer_Cocos2dx.hpp"
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


class CubismOffscreenSurface_Cocos2dx
{
public:

    CubismOffscreenSurface_Cocos2dx();

    void BeginDraw(CubismCommandBuffer_Cocos2dx* commandBuffer, cocos2d::Texture2D* colorBufferOnFinishDrawing);

    void EndDraw(CubismCommandBuffer_Cocos2dx* commandBuffer);

    void Clear(CubismCommandBuffer_Cocos2dx* commandBuffer, float r, float g, float b, float a);

    csmBool CreateOffscreenSurface(csmUint32 displayBufferWidth, csmUint32 displayBufferHeight, cocos2d::RenderTexture* renderTexture = NULL);

    void DestroyOffscreenSurface();

    cocos2d::Texture2D* GetColorBuffer() const;

    csmUint32 GetBufferWidth() const;

    csmUint32 GetBufferHeight() const;

    csmRectF GetViewPortSize() const;

    csmBool IsValid() const;

private:
    cocos2d::RenderTexture*      _renderTexture;
    cocos2d::Texture2D*          _colorBuffer;
    csmBool _isInheritedRenderTexture;

    cocos2d::Texture2D*      _previousColorBuffer;

    csmUint32   _bufferWidth;
    csmUint32   _bufferHeight;

    csmRectF _viewPortSize;
};


}}}}

