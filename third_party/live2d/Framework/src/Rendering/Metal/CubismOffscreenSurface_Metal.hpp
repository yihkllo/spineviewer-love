
#pragma once

#include <MetalKit/MetalKit.h>
#include "CubismFramework.hpp"

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

class CubismOffscreenSurface_Metal
{
public:

    CubismOffscreenSurface_Metal();

    csmBool CreateOffscreenSurface(csmUint32 displayBufferWidth, csmUint32 displayBufferHeight, id <MTLTexture> colorBuffer = NULL);

    void DestroyOffscreenSurface();

    id <MTLTexture> GetColorBuffer() const;

    void SetClearColor(float r, float g, float b, float a);

    csmUint32 GetBufferWidth() const;

    csmUint32 GetBufferHeight() const;

    csmBool IsValid() const;

    const MTLViewport* GetViewport() const;

    MTLRenderPassDescriptor* GetRenderPassDescriptor() const;

    void SetMTLPixelFormat(MTLPixelFormat pixelFormat);

private:
    id <MTLTexture>  _colorBuffer;
    MTLRenderPassDescriptor *_renderPassDescriptor;
    csmUint32   _bufferWidth;
    csmUint32   _bufferHeight;
    MTLViewport _viewPort;
    MTLPixelFormat _pixelFormat;
    float _clearColorR;
    float _clearColorG;
    float _clearColorB;
    float _clearColorA;
};

}}}}

