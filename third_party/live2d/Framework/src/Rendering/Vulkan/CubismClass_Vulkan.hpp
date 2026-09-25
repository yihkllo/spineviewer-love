

#pragma once
#include <fstream>
#include <vulkan/vulkan.h>
#include "CubismFramework.hpp"
#include "Type/csmVector.hpp"

namespace Live2D { namespace Cubism { namespace Framework {
class CubismBufferVulkan
{
public:
    CubismBufferVulkan();

    csmUint32 FindMemoryType(VkPhysicalDevice physicalDevice, csmUint32 typeFilter, VkMemoryPropertyFlags properties);

    void CreateBuffer(VkDevice device, VkPhysicalDevice physicalDevice, VkDeviceSize size, VkBufferUsageFlags usage,
                      VkMemoryPropertyFlags properties);

    void Map(VkDevice device, VkDeviceSize size);

    void MemCpy(const void* src, VkDeviceSize size) const;

    void UnMap(VkDevice device) const;

    void Destroy(VkDevice device);

    VkBuffer GetBuffer() const { return buffer; }

private:
    VkBuffer buffer;
    VkDeviceMemory memory;
    void* mapped;
};

class CubismImageVulkan
{
public:
    CubismImageVulkan();

    csmUint32 FindMemoryType(VkPhysicalDevice physicalDevice, csmUint32 typeFilter, VkMemoryPropertyFlags properties);

    void CreateImage(VkDevice device, VkPhysicalDevice physicalDevice,
                     csmInt32 w, csmInt32 h,
                     csmInt32 mipLevel, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage);

    void CreateView(VkDevice device, VkFormat format, VkImageAspectFlags aspectFlags, csmInt32 mipLevel);

    void CreateSampler(VkDevice device, float maxAnistropy, csmUint32 mipLevel);

    void SetImageLayout(VkCommandBuffer commandBuffer, VkImageLayout newLayout, csmUint32 mipLevels, VkImageAspectFlags aspectMask);

    void SetCurrentLayout(VkImageLayout newLayout);

    void Destroy(VkDevice device);

    VkImage GetImage() const { return image; }

    VkImageView GetView() const { return view; }

    VkSampler GetSampler() const { return sampler; }

    csmInt32 GetWidth() const { return width; }

    csmInt32 GetHeight() const { return height; }

private:
    VkImage image;
    VkDeviceMemory memory;
    VkImageView view;
    VkSampler sampler;
    VkImageLayout currentLayout;
    csmInt32 width, height;
};
}}}
