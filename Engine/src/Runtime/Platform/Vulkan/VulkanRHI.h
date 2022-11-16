#pragma once
#include "Runtime/Function/RHI/RHI.h"
#include "VulkanViewport.h"
#include "VulkanDevice.h"
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

        virtual uint32_t GetGfxQueueFamily() override;

        virtual void GfxQueueSubmit(const QueueSubmitDesc& desc) override;
        virtual void ComputeQueueSubmit(const QueueSubmitDesc& desc) override;
        virtual void TransferQueueSubmit(const QueueSubmitDesc& desc) override;
        
        virtual void Present(Semaphore** semaphores, uint32_t semaphore_count) override;

        virtual CommandBuffer* RHICreateCommandBuffer() override;
        virtual std::shared_ptr<RHITexture2D> RHICreateTexture2D(uint32_t width, uint32_t height, PixelFormat in_format, uint32_t miplevels = 1) override;
        virtual std::shared_ptr<RHITexture2D> RHICreateTexture2D(std::string_view path, uint32_t miplevels = 1) override;
        virtual renderer::RenderPass*         RHICreateRenderPass(const char* render_pass_name, const renderer::RenderPassDesc& desc,
                                                                  renderer::RenderPass::EXEC_FUNC exec) override;
        virtual renderer::RenderTarget* RHICreateRenderTarget(renderer::RenderPass& pass) override;
        virtual renderer::Pipeline*     RHICreatePipeline(const char* vert_path,
                                                          const char* frag_path,
                                                          const renderer::PipelineDesc& desc) override;
        virtual std::shared_ptr<RHIVertexBuffer> RHICreateVertexBuffer(uint64_t size) override;
        virtual std::shared_ptr<RHIStagingBuffer> RHICreateStagingBuffer(uint64_t size) override;

        virtual Semaphore* RHICreateSemaphore() override;
        virtual Fence* RHICreateFence() override;
        virtual void RHIDestroySemaphore(Semaphore* semaphore) override;
        virtual void RHIDestroyFence(Fence* fence) override;
        virtual void RHIWaitForFences(Fence** fence, uint32_t fence_count) override;

        void RHITick(float delta_time) override;
        void RHIBlockUntilGPUIdle() override;

        void GetExtensionsAndLayers();

        inline VkInstance GetVkInstance() const { return instance_; };
        inline VulkanDevice* GetDevice() { return device_; };
        inline VulkanViewport* GetViewport() { return viewport_; };

        VmaAllocator allocator_;
    private:
        void CreateInstance();

        // @brief Pick the first discrete device and initialize it
        void SelectAndInitDevice();

        void CreateVulkanMemoryAllocator();
    protected:
        VkInstance instance_ = VK_NULL_HANDLE;
        std::vector<const char*> instance_extensions_;
        std::vector<const char*> instance_layers_;

        VulkanDevice* device_ = nullptr;

        VulkanViewport* viewport_ = nullptr;
    };
}