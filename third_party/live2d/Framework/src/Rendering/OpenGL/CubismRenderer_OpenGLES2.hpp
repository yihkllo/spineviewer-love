

#pragma once

#include "../CubismRenderer.hpp"
#include "../CubismClippingManager.hpp"
#include "CubismFramework.hpp"
#include "CubismOffscreenSurface_OpenGLES2.hpp"
#include "CubismShader_OpenGLES2.hpp"
#include "Type/csmVector.hpp"
#include "Type/csmRectF.hpp"
#include "Math/CubismVector2.hpp"
#include "Type/csmMap.hpp"

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

class CubismRenderer_OpenGLES2;
class CubismClippingContext_OpenGLES2;
class CubismShader_OpenGLES2;

class CubismClippingManager_OpenGLES2 : public CubismClippingManager<CubismClippingContext_OpenGLES2, CubismOffscreenSurface_OpenGLES2>
{
public:

    void SetupClippingContext(CubismModel& model, CubismRenderer_OpenGLES2* renderer, GLint lastFBO, GLint lastViewport[4]);
};

class CubismClippingContext_OpenGLES2 : public CubismClippingContext
{
    friend class CubismClippingManager_OpenGLES2;
    friend class CubismRenderer_OpenGLES2;

public:
    CubismClippingContext_OpenGLES2(CubismClippingManager<CubismClippingContext_OpenGLES2, CubismOffscreenSurface_OpenGLES2>* manager, CubismModel& model, const csmInt32* clippingDrawableIndices, csmInt32 clipCount);

    virtual ~CubismClippingContext_OpenGLES2();

    CubismClippingManager<CubismClippingContext_OpenGLES2, CubismOffscreenSurface_OpenGLES2>* GetClippingManager();

    CubismClippingManager<CubismClippingContext_OpenGLES2, CubismOffscreenSurface_OpenGLES2>* _owner;
};

class CubismRendererProfile_OpenGLES2
{
    friend class CubismRenderer_OpenGLES2;

private:
    CubismRendererProfile_OpenGLES2() {};

    virtual ~CubismRendererProfile_OpenGLES2() {};

    void Save();

    void Restore();

    void SetGlEnable(GLenum index, GLboolean enabled);

    void SetGlEnableVertexAttribArray(GLuint index, GLint enabled);

    GLint _lastArrayBufferBinding;
    GLint _lastElementArrayBufferBinding;
    GLint _lastProgram;
    GLint _lastActiveTexture;
    GLint _lastTexture0Binding2D;
    GLint _lastTexture1Binding2D;
    GLint _lastVertexAttribArrayEnabled[4];
    GLboolean _lastScissorTest;
    GLboolean _lastBlend;
    GLboolean _lastStencilTest;
    GLboolean _lastDepthTest;
    GLboolean _lastCullFace;
    GLint _lastFrontFace;
    GLboolean _lastColorMask[4];
    GLint _lastBlending[4];
    GLint _lastFBO;
    GLint _lastViewport[4];
};

class CubismRenderer_OpenGLES2 : public CubismRenderer
{
    friend class CubismRenderer;
    friend class CubismClippingManager_OpenGLES2;
    friend class CubismShader_OpenGLES2;

public:
    void Initialize(Framework::CubismModel* model);

    void Initialize(Framework::CubismModel* model, csmInt32 maskBufferCount);

    void BindTexture(csmUint32 modelTextureIndex, GLuint glTextureIndex);

    const csmMap<csmInt32, GLuint>& GetBindedTextures() const;

    void SetClippingMaskBufferSize(csmFloat32 width, csmFloat32 height);

    csmInt32 GetRenderTextureCount() const;

    CubismVector2 GetClippingMaskBufferSize() const;

    CubismOffscreenSurface_OpenGLES2* GetMaskBuffer(csmInt32 index);

protected:
    CubismRenderer_OpenGLES2();

    virtual ~CubismRenderer_OpenGLES2();

    virtual void DoDrawModel() override;

    void DrawMeshOpenGL(const CubismModel& model, const csmInt32 index);

#ifdef CSM_TARGET_ANDROID_ES2
public:
    static void SetExtShaderMode(csmBool extMdoe, csmBool extPAMode = false);

    static void ReloadShader();
#endif

private:
    CubismRenderer_OpenGLES2(const CubismRenderer_OpenGLES2&);
    CubismRenderer_OpenGLES2& operator=(const CubismRenderer_OpenGLES2&);

    static void DoStaticRelease();

    void PreDraw();

    void PostDraw(){};

    virtual void SaveProfile();

    virtual void RestoreProfile();

    void SetClippingContextBufferForMask(CubismClippingContext_OpenGLES2* clip);

    CubismClippingContext_OpenGLES2* GetClippingContextBufferForMask() const;

    void SetClippingContextBufferForDraw(CubismClippingContext_OpenGLES2* clip);

    CubismClippingContext_OpenGLES2* GetClippingContextBufferForDraw() const;

    const csmBool inline IsGeneratingMask() const;

    GLuint GetBindedTextureId(csmInt32 textureId);

#ifdef CSM_TARGET_WIN_GL
    void  InitializeGlFunctions();

    void* WinGlGetProcAddress(const csmChar* name);

    void  CheckGlError(const csmChar* message);
#endif

    csmMap<csmInt32, GLuint> _textures;
    csmVector<csmInt32> _sortedDrawableIndexList;
    CubismRendererProfile_OpenGLES2 _rendererProfile;
    CubismClippingManager_OpenGLES2* _clippingManager;
    CubismClippingContext_OpenGLES2* _clippingContextBufferForMask;
    CubismClippingContext_OpenGLES2* _clippingContextBufferForDraw;

    csmVector<CubismOffscreenSurface_OpenGLES2>   _offscreenSurfaces;
};

}}}}
