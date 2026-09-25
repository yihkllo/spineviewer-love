

#pragma once
#include <string>
#include "../CubismRenderer.hpp"
#include "../CubismClippingManager.hpp"
#include <vulkan/vulkan.h>
#include "CubismFramework.hpp"
#include "CubismOffscreenSurface_Vulkan.hpp"
#include "CubismClass_Vulkan.hpp"
#include "Type/csmVector.hpp"
#include "Type/csmMap.hpp"
#include "Math/CubismVector2.hpp"

namespace Live2D { namespace Cubism { namespace Framework { namespace Rendering {

VkViewport GetViewport(csmFloat32 width, csmFloat32 height, csmFloat32 minDepth, csmFloat32 maxDepth);

VkRect2D GetScissor(csmFloat32 offsetX, csmFloat32 offsetY, csmFloat32 width, csmFloat32 height);

class CubismRenderer_Vulkan;
class CubismClippingContext_Vulkan;

class CubismClippingManager_Vulkan : public CubismClippingManager<
            CubismClippingContext_Vulkan, CubismOffscreenSurface_Vulkan>
{
public:
    void SetupClippingContext(CubismModel& model, VkCommandBuffer commandBuffer, VkCommandBuffer updateCommandBuffer,
                              CubismRenderer_Vulkan* renderer);
};

class CubismClippingContext_Vulkan : public CubismClippingContext
{
    friend class CubismClippingManager_Vulkan;
    friend class CubismRenderer_Vulkan;

public:
    CubismClippingContext_Vulkan(
        CubismClippingManager<CubismClippingContext_Vulkan, CubismOffscreenSurface_Vulkan>* manager, CubismModel& model,
        const csmInt32* clippingDrawableIndices, csmInt32 clipCount);

    virtual ~CubismClippingContext_Vulkan();

    CubismClippingManager<CubismClippingContext_Vulkan, CubismOffscreenSurface_Vulkan>* GetClippingManager();

    CubismClippingManager<CubismClippingContext_Vulkan, CubismOffscreenSurface_Vulkan>* _owner;
};

enum ShaderNames
{
    ShaderNames_SetupMask,

    ShaderNames_Normal,
    ShaderNames_NormalMasked,
    ShaderNames_NormalMaskedInverted,
    ShaderNames_NormalPremultipliedAlpha,
    ShaderNames_NormalMaskedPremultipliedAlpha,
    ShaderNames_NormalMaskedInvertedPremultipliedAlpha,

    ShaderNames_Add,
    ShaderNames_AddMasked,
    ShaderNames_AddMaskedInverted,
    ShaderNames_AddPremultipliedAlpha,
    ShaderNames_AddMaskedPremultipliedAlpha,
    ShaderNames_AddMaskedPremultipliedAlphaInverted,

    ShaderNames_Mult,
    ShaderNames_MultMasked,
    ShaderNames_MultMaskedInverted,
    ShaderNames_MultPremultipliedAlpha,
    ShaderNames_MultMaskedPremultipliedAlpha,
    ShaderNames_MultMaskedPremultipliedAlphaInverted,
};

enum Blend
{
    Blend_Normal,
    Blend_Add,
    Blend_Mult,
    Blend_Mask
};

struct ModelVertex
{
    CubismVector2 pos;
    CubismVector2 texCoord;

    static VkVertexInputBindingDescription GetBindingDescription()
    {
        VkVertexInputBindingDescription bindingDescription{};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(ModelVertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescription;
    }

    static void GetAttributeDescriptions(VkVertexInputAttributeDescription attributeDescriptions[2])
    {
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(ModelVertex, pos);

        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(ModelVertex, texCoord);
    }
};

class CubismPipeline_Vulkan
{
public:
    CubismPipeline_Vulkan();

    ~CubismPipeline_Vulkan();

    struct PipelineResource
    {
        VkShaderModule CreateShaderModule(VkDevice device, std::string filename);

        void CreateGraphicsPipeline(std::string vertFileName, std::string fragFileName,
                                    VkDescriptorSetLayout descriptorSetLayout);
        void Release();
        VkPipeline GetPipeline(csmInt32 index) { return _pipeline[index]; }
        VkPipelineLayout GetPipelineLayout(csmInt32 index) { return _pipelineLayout[index]; }

    private:
        csmVector<VkPipeline> _pipeline;
        csmVector<VkPipelineLayout> _pipelineLayout;
    };

    void CreatePipelines(VkDescriptorSetLayout descriptorSetLayout);

    VkPipeline GetPipeline(csmInt32 shaderIndex, csmInt32 blendIndex)
    {
        return _pipelineResource[shaderIndex]->GetPipeline(blendIndex);
    }

    VkPipelineLayout GetPipelineLayout(csmInt32 shaderIndex, csmInt32 blendIndex)
    {
        return _pipelineResource[shaderIndex]->GetPipelineLayout(blendIndex);
    }

    static CubismPipeline_Vulkan* GetInstance();

    void ReleaseShaderProgram();

private:
    csmVector<PipelineResource*> _pipelineResource;
};

class CubismRenderer_Vulkan : public CubismRenderer
{
    friend class CubismClippingManager_Vulkan;
    friend class CubismRenderer;

    struct ModelUBO
    {
        csmFloat32 projectionMatrix[16];
        csmFloat32 clipMatrix[16];
        csmFloat32 baseColor[4];
        csmFloat32 multiplyColor[4];
        csmFloat32 screenColor[4];
        csmFloat32 channelFlag[4];
    };

    struct Descriptor
    {
        CubismBufferVulkan uniformBuffer;
        VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
        bool isDescriptorSetUpdated = false;
        VkDescriptorSet descriptorSetMasked = VK_NULL_HANDLE;
        bool isDescriptorSetMaskedUpdated = false;
    };

protected:
    CubismRenderer_Vulkan();

    ~CubismRenderer_Vulkan() override;

    static void DoStaticRelease();

public:

    VkCommandBuffer BeginSingleTimeCommands();

    void SubmitCommand(VkCommandBuffer commandBuffer, VkSemaphore signalUpdateFinishedSemaphore = VK_NULL_HANDLE, VkSemaphore waitUpdateFinishedSemaphore = VK_NULL_HANDLE);

    static void InitializeConstantSettings(VkDevice device, VkPhysicalDevice physicalDevice,
                                           VkCommandPool commandPool, VkQueue queue,
                                           VkExtent2D extent, VkFormat depthFormat, VkFormat surfaceFormat,
                                           VkImage swapchainImage,
                                           VkImageView swapchainImageView,
                                           VkFormat dFormat);

    static void EnableChangeRenderTarget();

    static void SetRenderTarget(VkImage image, VkImageView imageview);

    static void UpdateSwapchainVariable(VkExtent2D extent, VkImage image,
                                        VkImageView imageView);

    static void UpdateRendererSettings(VkImage image, VkImageView imageView);

    void CreateCommandBuffer();


    void CreateVertexBuffer();

    void CreateIndexBuffer();

    void CreateDescriptorSets();

    void CreateDepthBuffer();

    void InitializeRenderer();

    void Initialize(Framework::CubismModel* model) override;

    void Initialize(Framework::CubismModel* model, csmInt32 maskBufferCount) override;

    void CopyToBuffer(csmInt32 drawAssign, const csmInt32 vcount, const csmFloat32* varray, const csmFloat32* uvarray,
                      VkCommandBuffer commandBuffer);

    static void UpdateMatrix(csmFloat32 vkMat4[16], CubismMatrix44 cubismMat);

    void UpdateColor(csmFloat32 vkVec4[4], csmFloat32 r, csmFloat32 g, csmFloat32 b, csmFloat32 a);

    void UpdateDescriptorSet(Descriptor& descriptor, csmUint32 textureIndex, bool isMasked);

    void ExecuteDrawForDraw(const CubismModel& model, const csmInt32 index, VkCommandBuffer& cmdBuffer);

    void ExecuteDrawForMask(const CubismModel& model, const csmInt32 index, VkCommandBuffer& cmdBuffer);

    void DrawMeshVulkan(const CubismModel& model, const csmInt32 index,
                        VkCommandBuffer commandBuffer, VkCommandBuffer updateCommandBuffer);

    void BeginRendering(VkCommandBuffer drawCommandBuffer, bool isResume);

    void EndRendering(VkCommandBuffer drawCommandBuffer);

    void DoDrawModel() override;

    void SaveProfile() override {}

    void RestoreProfile() override {}

    void SetClippingContextBufferForMask(CubismClippingContext_Vulkan* clip);

    CubismClippingContext_Vulkan* GetClippingContextBufferForMask() const;

    void SetClippingContextBufferForDraw(CubismClippingContext_Vulkan* clip);

    CubismClippingContext_Vulkan* GetClippingContextBufferForDraw() const;

    void BindTexture(CubismImageVulkan& image);

    void SetClippingMaskBufferSize(csmFloat32 width, csmFloat32 height);

    CubismVector2 GetClippingMaskBufferSize() const;

    CubismOffscreenSurface_Vulkan* GetMaskBuffer(csmInt32 index);

private:

    void SetColorUniformBuffer(ModelUBO& ubo, const CubismTextureColor& baseColor,
                               const CubismTextureColor& multiplyColor, const CubismTextureColor& screenColor);

    void BindVertexAndIndexBuffers(const csmInt32 index, VkCommandBuffer& cmdBuffer);

    void SetColorChannel(ModelUBO& ubo, CubismClippingContext_Vulkan* contextBuffer);

    PFN_vkCmdSetCullModeEXT vkCmdSetCullModeEXT;

    CubismRenderer_Vulkan(const CubismRenderer_Vulkan&);
    CubismRenderer_Vulkan& operator=(const CubismRenderer_Vulkan&);

    CubismClippingManager_Vulkan* _clippingManager;
    csmVector<csmInt32> _sortedDrawableIndexList;
    CubismClippingContext_Vulkan* _clippingContextBufferForMask;
    CubismClippingContext_Vulkan* _clippingContextBufferForDraw;
    csmVector<CubismOffscreenSurface_Vulkan> _offscreenFrameBuffers;
    csmVector<CubismBufferVulkan> _vertexBuffers;
    csmVector<CubismBufferVulkan> _stagingBuffers;
    csmVector<CubismBufferVulkan> _indexBuffers;
    VkDescriptorPool _descriptorPool;
    VkDescriptorSetLayout _descriptorSetLayout;
    csmVector<Descriptor> _descriptorSets;
    csmVector<CubismImageVulkan> _textures;
    CubismImageVulkan _depthImage;
    VkClearValue _clearColor;
    VkSemaphore _updateFinishedSemaphore;
    VkCommandBuffer updateCommandBuffer;
    VkCommandBuffer drawCommandBuffer;
};
}}}}

