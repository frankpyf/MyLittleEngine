#pragma once
#include "RHIResource.h"
#include "CommandBuffer.h"
#include "RenderPass.h"
#include "Descriptor.h"
// Forward declaration
struct ImDrawData;

namespace rhi{
    struct Semaphore {};

    struct Fence {};

    struct QueueSubmitDesc
    {
        RHIEncoderBase** encoders;
        uint32_t cmds_count = 0;
        Fence* signal_fence = nullptr;
        Semaphore** wait_semaphore;
        uint32_t wait_semaphore_count = 0;
        Semaphore** signal_semaphore;
        uint32_t signal_semaphore_count = 0;
    };

    class RHI
    {
    public:
        enum class GfxAPI
        {
            None = 0, Vulkan = 1
        };
    public:
        virtual ~RHI() = default;

        virtual void Init() = 0;
        virtual void Shutdown() = 0;

        virtual void RHITick(float delta_time) = 0;
        virtual void RHIBlockUntilGPUIdle() = 0;

        virtual void* GetNativeInstance() = 0;
        virtual void* GetNativeDevice() = 0;
        virtual void* GetNativePhysicalDevice() = 0;
        virtual void* GetNativeGraphicsQueue() = 0;
        virtual void* GetNativeComputeQueue() = 0;

        // Viewport, Swapchain stuff
        virtual void AcquireNextImage(Semaphore* semaphore) {};
        virtual void* GetNativeSwapchainImageView() = 0;
        virtual uint32_t GetViewportWidth() = 0;
        virtual uint32_t GetViewportHeight() = 0;

        virtual uint32_t GetGfxQueueFamily() = 0;
        
        virtual void GfxQueueSubmit(const QueueSubmitDesc& desc) = 0;
        virtual void ComputeQueueSubmit(const QueueSubmitDesc& desc) = 0;
        virtual void TransferQueueSubmit(const QueueSubmitDesc& desc) = 0;

        virtual void Present(Semaphore** semaphores, uint32_t semaphore_count) = 0;

        [[nodiscard]] virtual DescriptorSetPtr RHICreateDescriptorSet() = 0;
        [[nodiscard]] virtual DescriptorSetLayoutCachePtr CreateDescriptorSetLayoutCache() = 0;
        [[nodiscard]] virtual DescriptorAllocatorPtr CreateDescriptorAllocator() = 0;

        virtual CommandBuffer* RHICreateCommandBuffer() = 0;
        virtual std::unique_ptr<RenderPass> RHICreateRenderPass(const RenderPass::Descriptor& desc) = 0;
        virtual std::unique_ptr<RenderTarget> RHICreateRenderTarget(const RenderTarget::Descriptor& desc) = 0;
        
        [[nodiscard]] virtual ShaderModule* RHICreateShaderModule(const char* path) = 0;
        virtual void RHIFreeShaderModule(ShaderModule& shader) = 0;
        [[nodiscard]] virtual PipelineLayout* RHICreatePipelineLayout(const PipelineLayout::Descriptor& desc) = 0;
        virtual void RHIFreePipelineLayout(PipelineLayout& layout) = 0;
        [[nodiscard]] virtual PipelineRef RHICreatePipeline(const RHIPipeline::Descriptor& desc) = 0;
        virtual void RHIFreePipeline(RHIPipeline& pipeline) = 0;
        [[nodiscard]] virtual BufferRef RHICreateBuffer(const RHIBuffer::Descriptor& desc) = 0;
        virtual void RHIFreeBuffer(RHIBuffer& buffer) = 0;
        [[nodiscard]] virtual TextureRef RHICreateTexture(const RHITexture::Descriptor& desc) = 0;
        virtual void ResizeTexture(RHITexture& texture, uint32_t width, uint32_t height) = 0;
        virtual void RHIFreeTexture(RHITexture& texture) = 0;

        virtual Semaphore* RHICreateSemaphore() = 0;
        virtual Fence* RHICreateFence() = 0;
        virtual void RHIDestroySemaphore(Semaphore* semaphore) = 0;
        virtual void RHIDestroyFence(Fence* fence) = 0;
        // Block until the condition is satisfied, then reset the fence
        virtual void RHIWaitForFences(Fence** fence, uint32_t fence_count) = 0;
        virtual bool RHIIsFenceReady(Fence* fence) = 0;

        static GfxAPI GetAPI() { return api_; }
        static RHI& GetRHIInstance();
    private:
        static GfxAPI api_;
    };
} // namespace engine
