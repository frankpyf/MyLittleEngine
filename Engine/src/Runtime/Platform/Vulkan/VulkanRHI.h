#pragma once
#include "Runtime/Function/RHI/RHI.h"
#include "VulkanViewport.h"
#include "VulkanDevice.h"
#include "VulkanResource.h"
#include "Runtime/Core/Base/Singleton.h"

#include "vk_mem_alloc.h"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
namespace rhi {
    struct VulkanSemaphore :public Semaphore {
        VkSemaphore semaphore = VK_NULL_HANDLE;
    };
    struct VulkanFence :public Fence {
        VkFence fence = VK_NULL_HANDLE;
    };

    class VulkanRHI : public RHI
    {
        friend class VulkanRenderer;
    public:
        virtual void Init() override;
        virtual void Shutdown() override;

        virtual void* GetNativeInstance() override;
        virtual void* GetNativeDevice() override;
        virtual void* GetNativePhysicalDevice() override;
        virtual void* GetNativeGraphicsQueue() override;
        virtual void* GetNativeComputeQueue() override;

        virtual void AcquireNextImage(Semaphore* semaphore) override;
        virtual void* GetNativeSwapchainImageView() override;
        virtual uint32_t GetViewportWidth() override;
        virtual uint32_t GetViewportHeight() override;

        VkFormat GetSwapchainImageFormat() { return viewport_->GetSwapChain()->GetImageFormat(); };

        virtual uint32_t GetGfxQueueFamily() override;

        virtual void GfxQueueSubmit(const QueueSubmitDesc& desc) override;
        virtual void ComputeQueueSubmit(const QueueSubmitDesc& desc) override;
        virtual void TransferQueueSubmit(const QueueSubmitDesc& desc) override;
        
        virtual void Present(Semaphore** semaphores, uint32_t semaphore_count) override;

        [[nodiscard]] virtual DescriptorSetPtr RHICreateDescriptorSet() override;
        [[nodiscard]] virtual DescriptorSetLayoutCachePtr CreateDescriptorSetLayoutCache() override;
        [[nodiscard]] virtual DescriptorAllocatorPtr CreateDescriptorAllocator() override;

        virtual CommandBuffer* RHICreateCommandBuffer() override;
        virtual std::unique_ptr<RenderPass>   RHICreateRenderPass(const RenderPass::Descriptor& desc) override;
        virtual std::unique_ptr<RenderTarget> RHICreateRenderTarget(const RenderTarget::Descriptor& desc) override;
        
        [[nodiscard]] virtual ShaderModule* RHICreateShaderModule(const char* path) override;
        virtual void RHIFreeShaderModule(ShaderModule& shader) override;
        [[nodiscard]] virtual PipelineLayout* RHICreatePipelineLayout(const PipelineLayout::Descriptor& desc) override;
        virtual void RHIFreePipelineLayout(PipelineLayout& layout) override;
        [[nodiscard]] virtual PipelineRef RHICreatePipeline(const RHIPipeline::Descriptor& desc) override;
        virtual void RHIFreePipeline(RHIPipeline& pipeline) override;
        [[nodiscard]] virtual BufferRef RHICreateBuffer(const RHIBuffer::Descriptor& desc) override;
        virtual void RHIFreeBuffer(RHIBuffer& buffer) override;
        [[nodiscard]] virtual TextureRef RHICreateTexture(const RHITexture::Descriptor& desc) override;
        virtual void ResizeTexture(RHITexture& texture, uint32_t width, uint32_t height) override;
        virtual void RHIFreeTexture(RHITexture& texture) override;

        virtual Semaphore* RHICreateSemaphore() override;
        virtual Fence* RHICreateFence() override;
        virtual void RHIDestroySemaphore(Semaphore* semaphore) override;
        virtual void RHIDestroyFence(Fence* fence) override;
        virtual void RHIWaitForFences(Fence** fence, uint32_t fence_count) override;
        virtual bool RHIIsFenceReady(Fence* fence) override;

        void RHITick(float delta_time) override;
        void RHIBlockUntilGPUIdle() override;

        void GetExtensionsAndLayers();

        inline VkInstance GetVkInstance() const { return instance_; };
        inline VulkanDevice* GetDevice() { return device_; };
        inline VulkanViewport* GetViewport() { return viewport_; };

        inline VkFormat GetDepthFormat() { return depth_format_; };

        VmaAllocator allocator_;
    private:
        void CreateInstance();

        // @brief Pick the first discrete device and initialize it
        void SelectAndInitDevice();

        void CreateVulkanMemoryAllocator();

        void AllocateTextureMemory(VulkanTexture* texture);
    protected:
        VkInstance instance_ = VK_NULL_HANDLE;
        std::vector<const char*> instance_extensions_;
        std::vector<const char*> instance_layers_;

        VulkanDevice* device_ = nullptr;

        VulkanViewport* viewport_ = nullptr;

        VkFormat depth_format_;
    };
}