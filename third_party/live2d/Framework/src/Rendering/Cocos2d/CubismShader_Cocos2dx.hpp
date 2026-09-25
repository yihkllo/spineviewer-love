

#pragma once

#include "CubismFramework.hpp"
#include "CubismCommandBuffer_Cocos2dx.hpp"
#include "CubismRenderer_Cocos2dx.hpp"
#include "Type/csmVector.hpp"

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

class CubismShader_Cocos2dx
{
public:
    static CubismShader_Cocos2dx* GetInstance();

    static void DeleteInstance();

    void SetupShaderProgramForMask(CubismCommandBuffer_Cocos2dx::DrawCommandBuffer::DrawCommand* drawCommand, CubismRenderer_Cocos2dx* renderer
                                ,const CubismModel& model, const csmInt32 index);

    void SetupShaderProgramForDraw(CubismCommandBuffer_Cocos2dx::DrawCommandBuffer::DrawCommand* drawCommand, CubismRenderer_Cocos2dx* renderer
                                ,const CubismModel& model, const csmInt32 index);

private:
    struct CubismShaderSet
    {
        cocos2d::backend::Program* ShaderProgram;
        unsigned int AttributePositionLocation;
        unsigned int AttributeTexCoordLocation;
        cocos2d::backend::UniformLocation UniformMatrixLocation;
        cocos2d::backend::UniformLocation UniformClipMatrixLocation;
        cocos2d::backend::UniformLocation SamplerTexture0Location;
        cocos2d::backend::UniformLocation SamplerTexture1Location;
        cocos2d::backend::UniformLocation UniformBaseColorLocation;
        cocos2d::backend::UniformLocation UniformMultiplyColorLocation;
        cocos2d::backend::UniformLocation UniformScreenColorLocation;
        cocos2d::backend::UniformLocation UnifromChannelFlagLocation;
    };

    CubismShader_Cocos2dx();

    virtual ~CubismShader_Cocos2dx();

    void ReleaseShaderProgram();

    void GenerateShaders();

    cocos2d::backend::Program* LoadShaderProgram(const csmChar* vertShaderSrc, const csmChar* fragShaderSrc);

    void SetVertexAttributes(cocos2d::backend::ProgramState* programState, CubismShaderSet* shaderSet);

    void SetupTexture(CubismRenderer_Cocos2dx* renderer, cocos2d::backend::ProgramState* programState
                    , const CubismModel& model, const csmInt32 index, CubismShaderSet* shaderSet);

    void SetColorUniformVariables(cocos2d::backend::ProgramState* programState, CubismShaderSet* shaderSet, CubismRenderer::CubismTextureColor& baseColor
                                , CubismRenderer::CubismTextureColor& multiplyColor, CubismRenderer::CubismTextureColor& screenColor);

    void SetColorChannel(CubismRenderer_Cocos2dx* renderer, cocos2d::backend::ProgramState* programState,
                         CubismShaderSet* shaderSet, CubismClippingContext_Cocos2dx* contextBuffer);

#ifdef CSM_TARGET_ANDROID_ES2
public:
    static void SetExtShaderMode(csmBool extMode, csmBool extPAMode);

private:
    static csmBool  s_extMode;
    static csmBool  s_extPAMode;
#endif

    csmVector<CubismShaderSet*> _shaderSets;
};

}}}}
