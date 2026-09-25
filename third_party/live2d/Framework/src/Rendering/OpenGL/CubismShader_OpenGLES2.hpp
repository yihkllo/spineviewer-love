

#pragma once

#include "CubismFramework.hpp"
#include "CubismRenderer_OpenGLES2.hpp"

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

class CubismShader_OpenGLES2
{
public:
    static CubismShader_OpenGLES2* GetInstance();

    static void DeleteInstance();

    void SetupShaderProgramForDraw(CubismRenderer_OpenGLES2* renderer, const CubismModel& model, const csmInt32 index);

    void SetupShaderProgramForMask(CubismRenderer_OpenGLES2* renderer, const CubismModel& model, const csmInt32 index);

private:
    struct CubismShaderSet
    {
        GLuint ShaderProgram;
        GLuint AttributePositionLocation;
        GLuint AttributeTexCoordLocation;
        GLint UniformMatrixLocation;
        GLint UniformClipMatrixLocation;
        GLint SamplerTexture0Location;
        GLint SamplerTexture1Location;
        GLint UniformBaseColorLocation;
        GLint UniformMultiplyColorLocation;
        GLint UniformScreenColorLocation;
        GLint UnifromChannelFlagLocation;
    };

    CubismShader_OpenGLES2();

    virtual ~CubismShader_OpenGLES2();

    void ReleaseShaderProgram();

    void GenerateShaders();

    GLuint LoadShaderProgram(const csmChar* vertShaderSrc, const csmChar* fragShaderSrc);

    csmBool CompileShaderSource(GLuint* outShader, GLenum shaderType, const csmChar* shaderSource);

    csmBool LinkProgram(GLuint shaderProgram);

    csmBool ValidateProgram(GLuint shaderProgram);

    void SetVertexAttributes(const CubismModel& model, const csmInt32 index, CubismShaderSet* shaderSet);

    void SetupTexture(CubismRenderer_OpenGLES2* renderer, const CubismModel& model, const csmInt32 index, CubismShaderSet* shaderSet);

    void SetColorUniformVariables(CubismRenderer_OpenGLES2* renderer, const CubismModel& model, const csmInt32 index, CubismShaderSet* shaderSet,
                                  CubismRenderer::CubismTextureColor& baseColor, CubismRenderer::CubismTextureColor& multiplyColor, CubismRenderer::CubismTextureColor& screenColor);

    void SetColorChannelUniformVariables(CubismShaderSet* shaderSet, CubismClippingContext_OpenGLES2* contextBuffer);

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
