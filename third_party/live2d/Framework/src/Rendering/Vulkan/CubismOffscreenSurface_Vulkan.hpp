

#pragma once
#include <vulkan/vulkan.h>
#include "CubismFramework.hpp"
#include "CubismClass_Vulkan.hpp"

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {
class CubismOffscreenSurface_Vulkan
{
public:
    CubismOffscreenSurface_Vulkan();

    void BeginDraw(VkCommandBuffer commandBuffer, csmFloat32 r, csmFloat32 g, csmFloat32 b, csmFloat32 a);

    void EndDraw(VkCommandBuffer commandBuffer);

    void CreateOffscreenSurface(
        VkDevice device, VkPhysicalDevice physicalDevice,
        csmUint32 displayBufferWidth, csmUint32 displayBufferHeight,
        VkFormat surfaceFormat, VkFormat depthFormat
    );

    void DestroyOffscreenSurface(VkDevice device);

    VkImage GetTextureImage() const;

    VkImageView GetTextureView() const;

    VkSampler GetTextureSampler() const;

    csmUint32 GetBufferWidth() const;

    csmUint32 GetBufferHeight() const;

    bool IsValid() const;

private:
    csmUint32 _bufferWidth;
    csmUint32 _bufferHeight;
    CubismImageVulkan* _colorImage = VK_NULL_HANDLE;
    CubismImageVulkan* _depthImage = VK_NULL_HANDLE;
};
}}}}

