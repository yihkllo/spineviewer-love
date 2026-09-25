
#pragma once

#ifndef MetalShaderTypes_h
#define MetalShaderTypes_h

#include <simd/simd.h>

typedef enum MetalVertexInputIndex
{
    MetalVertexInputIndexVertices = 0,
    MetalVertexInputUVs = 1,
    MetalVertexInputIndexUniforms = 2,
} MetalVertexInputIndex;

typedef struct
{
    simd::float4x4 matrix;
    simd::float4x4 clipMatrix;
    vector_float4 channelFlag;
    vector_float4 baseColor;
    vector_float4 multiplyColor;
    vector_float4 screenColor;

} CubismShaderUniforms;


#endif
