
#pragma once

#include <MetalKit/MetalKit.h>
#include "CubismFramework.hpp"

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

class CubismOffscreenSurface_Metal;

class CubismCommandBuffer_Metal
{
public:
    class DrawCommandBuffer
    {
    public:
        class DrawCommand
        {
        public:
            DrawCommand();
            virtual ~DrawCommand();

            id <MTLCommandBuffer> GetMTLCommandBuffer();
            void SetMTLCommandBuffer(id <MTLCommandBuffer> commandBuffer);

            id <MTLRenderPipelineState> GetRenderPipelineState();
            void SetRenderPipelineState(id <MTLRenderPipelineState> renderPipelineState);

        private:
            DrawCommand& operator=(const DrawCommand&);
            id <MTLCommandBuffer> _mtlCommandBuffer;
            MTLRenderPassDescriptor* _renderPassDescriptor;
            id <MTLRenderPipelineState> _pipelineState;
        };

        DrawCommandBuffer();
        virtual ~DrawCommandBuffer();

        void CreateVertexBuffer(id<MTLDevice> device, csmSizeInt stride, csmSizeInt count);

        void CreateIndexBuffer(id<MTLDevice> device, csmSizeInt count);

        void UpdateVertexBuffer(void* data, void* uvData, csmSizeInt count);

        void UpdateIndexBuffer(void* data, csmSizeInt count);

        void SetCommandBuffer(id <MTLCommandBuffer> commandBuffer);

        DrawCommand* GetCommandDraw();
        id <MTLBuffer> GetVertexBuffer();
        id <MTLBuffer> GetUvBuffer();
        id <MTLBuffer> GetIndexBuffer();

    private:
        DrawCommand _drawCommandDraw;
        csmSizeInt _vbStride;
        csmSizeInt _vbCount;
        csmSizeInt _ibCount;
        csmUint8* _drawBuffer;
        id <MTLBuffer> _vertices;
        id <MTLBuffer> _uvs;
        id <MTLBuffer> _indices;
    };

    CubismCommandBuffer_Metal();
    virtual ~CubismCommandBuffer_Metal();
};

}}}}
