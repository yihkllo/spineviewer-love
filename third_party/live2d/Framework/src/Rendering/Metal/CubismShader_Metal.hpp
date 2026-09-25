

#pragma once

#include <MetalKit/MetalKit.h>
#include "CubismFramework.hpp"
#include "CubismRenderer_Metal.hpp"
#include "CubismCommandBuffer_Metal.hpp"
#include "Type/csmVector.hpp"
#include "MetalShaderTypes.h"

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

class CubismRenderer_Metal;

class CubismShader_Metal
{
public:
    static CubismShader_Metal* GetInstance();

    static void DeleteInstance();

    void SetupShaderProgramForDraw(CubismCommandBuffer_Metal::DrawCommandBuffer* drawCommandBuffer, id <MTLRenderCommandEncoder> renderEncoder
                                , CubismRenderer_Metal* renderer, const CubismModel& model, const csmInt32 index);

    void SetupShaderProgramForMask(CubismCommandBuffer_Metal::DrawCommandBuffer* drawCommandBuffer, id <MTLRenderCommandEncoder> renderEncoder
                                , CubismRenderer_Metal* renderer, const CubismModel& model, const csmInt32 index);

private:
    struct ShaderProgram
    {
        id <MTLFunction> vertexFunction;
        id <MTLFunction> fragmentFunction;
    };

    struct CubismShaderSet
    {
        ShaderProgram *ShaderProgram;
        id<MTLRenderPipelineState> RenderPipelineState;
        id<MTLDepthStencilState> DepthStencilState;
        id<MTLSamplerState> SamplerState;
    };

    CubismShader_Metal();

    virtual ~CubismShader_Metal();

    void GenerateShaders(CubismRenderer_Metal* renderer);

    simd::float4x4 ConvertCubismMatrix44IntoSimdFloat4x4(CubismMatrix44& matrix);

    void SetColorChannel(CubismShaderUniforms& shaderUniforms, CubismClippingContext_Metal* contextBuffer);

    void SetFragmentModelTexture(CubismCommandBuffer_Metal::DrawCommandBuffer* drawCommandBuffer, id <MTLRenderCommandEncoder> renderEncoder
                          , CubismRenderer_Metal* renderer, const CubismModel& model, const csmInt32 index);

    void SetVertexBufferForVerticesAndUvs(CubismCommandBuffer_Metal::DrawCommandBuffer* drawCommandBuffer, id <MTLRenderCommandEncoder> renderEncoder);

    ShaderProgram* LoadShaderProgram(const csmChar* vertShaderSrc, const csmChar* fragShaderSrc);
    id<MTLRenderPipelineState> MakeRenderPipelineState(id<MTLDevice> device, ShaderProgram* shaderProgram, int blendMode);
    id<MTLDepthStencilState> MakeDepthStencilState(id<MTLDevice> device);
    id<MTLSamplerState> MakeSamplerState(id<MTLDevice> device, CubismRenderer_Metal* renderer);

    id<MTLLibrary> _shaderLib;

    csmVector<CubismShaderSet*> _shaderSets;

};

}}}}
