

#pragma once

#include "../CubismRenderer.hpp"
#include "../CubismClippingManager.hpp"
#include "CubismFramework.hpp"
#include "CubismOffscreenSurface_Cocos2dx.hpp"
#include "CubismCommandBuffer_Cocos2dx.hpp"
#include "CubismShader_Cocos2dx.hpp"
#include "Math/CubismVector2.hpp"
#include "Type/csmVector.hpp"
#include "Type/csmRectF.hpp"
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

class CubismRenderer_Cocos2dx;
class CubismClippingContext_Cocos2dx;
class CubismShader_Cocos2dx;

class CubismClippingManager_Cocos2dx : public CubismClippingManager<CubismClippingContext_Cocos2dx, CubismOffscreenSurface_Cocos2dx>
{
public:

    void SetupClippingContext(CubismModel& model, CubismRenderer_Cocos2dx* renderer, cocos2d::Texture2D* lastColorBuffer, csmRectF lastViewport);
};

class CubismClippingContext_Cocos2dx : public CubismClippingContext
{
    friend class CubismClippingManager_Cocos2dx;
    friend class CubismRenderer_Cocos2dx;

public:
    CubismClippingContext_Cocos2dx(CubismClippingManager<CubismClippingContext_Cocos2dx, CubismOffscreenSurface_Cocos2dx>* manager, CubismModel& model, const csmInt32* clippingDrawableIndices, csmInt32 clipCount);

    virtual ~CubismClippingContext_Cocos2dx();

    CubismClippingManager<CubismClippingContext_Cocos2dx, CubismOffscreenSurface_Cocos2dx>* GetClippingManager();

private:
    csmVector<CubismCommandBuffer_Cocos2dx::DrawCommandBuffer*>* _clippingCommandBufferList;
    CubismClippingManager<CubismClippingContext_Cocos2dx, CubismOffscreenSurface_Cocos2dx>* _owner;
};

class CubismRendererProfile_Cocos2dx
{
    friend class CubismRenderer_Cocos2dx;

private:
    CubismRendererProfile_Cocos2dx() {};

    virtual ~CubismRendererProfile_Cocos2dx() {};

    void Save();

    void Restore();

    csmBool _lastScissorTest;
    csmBool _lastBlend;
    csmBool _lastStencilTest;
    csmBool _lastDepthTest;
    cocos2d::CullMode _lastCullFace;
    cocos2d::Winding _lastWinding;
    cocos2d::Texture2D* _lastColorBuffer;
    cocos2d::Texture2D* _lastDepthBuffer;
    cocos2d::Texture2D* _lastStencilBuffer;
    cocos2d::RenderTargetFlag _lastRenderTargetFlag;
    csmRectF _lastViewport;
};

class CubismRenderer_Cocos2dx : public CubismRenderer
{
    friend class CubismRenderer;
    friend class CubismClippingManager_Cocos2dx;
    friend class CubismShader_Cocos2dx;

public:
    void Initialize(Framework::CubismModel* model) override;

    void Initialize(Framework::CubismModel* model, csmInt32 maskBufferCount) override;

    void BindTexture(csmUint32 modelTextureIndex, cocos2d::Texture2D* texture);

    const csmMap<csmInt32, cocos2d::Texture2D*>& GetBindedTextures() const;

    void SetClippingMaskBufferSize(csmFloat32 width, csmFloat32 height);

    csmInt32 GetRenderTextureCount() const;

    CubismVector2 GetClippingMaskBufferSize() const;

    CubismOffscreenSurface_Cocos2dx* GetOffscreenSurface(csmInt32 index);

    static CubismCommandBuffer_Cocos2dx* GetCommandBuffer();

    static void StartFrame(CubismCommandBuffer_Cocos2dx* commandBuffer);

    static void EndFrame(CubismCommandBuffer_Cocos2dx* commandBuffer);

protected:
    CubismRenderer_Cocos2dx();

    virtual ~CubismRenderer_Cocos2dx();

    void DoDrawModel() override;

    void DrawMeshCocos2d(CubismCommandBuffer_Cocos2dx::DrawCommandBuffer::DrawCommand* drawCommand
                        ,const CubismModel& model, const csmInt32 index);

    CubismCommandBuffer_Cocos2dx::DrawCommandBuffer* GetDrawCommandBufferData(csmInt32 drawableIndex);

#ifdef CSM_TARGET_ANDROID_ES2
public:
    static void SetExtShaderMode(csmBool extMdoe, csmBool extPAMode = false);

    static void ReloadShader();
#endif

private:
    CubismRenderer_Cocos2dx(const CubismRenderer_Cocos2dx&);
    CubismRenderer_Cocos2dx& operator=(const CubismRenderer_Cocos2dx&);

    static void DoStaticRelease();

    void PreDraw();

    void PostDraw(){};

    void SaveProfile() override;

    void RestoreProfile() override;

    void SetClippingContextBufferForMask(CubismClippingContext_Cocos2dx* clip);

    CubismClippingContext_Cocos2dx* GetClippingContextBufferForMask() const;

    void SetClippingContextBufferForDraw(CubismClippingContext_Cocos2dx* clip);

    CubismClippingContext_Cocos2dx* GetClippingContextBufferForDraw() const;

    const csmBool inline IsGeneratingMask() const;

    cocos2d::Texture2D* GetBindedTexture(csmInt32 textureIndex);


    csmMap<csmInt32, cocos2d::Texture2D*> _textures;
    csmVector<csmInt32> _sortedDrawableIndexList;
    CubismRendererProfile_Cocos2dx _rendererProfile;
    CubismClippingManager_Cocos2dx* _clippingManager;
    CubismClippingContext_Cocos2dx* _clippingContextBufferForMask;
    CubismClippingContext_Cocos2dx* _clippingContextBufferForDraw;

    csmVector<CubismOffscreenSurface_Cocos2dx> _offscreenSurfaces;
    csmVector<CubismCommandBuffer_Cocos2dx::DrawCommandBuffer*> _drawableDrawCommandBuffer;
};

}}}}
