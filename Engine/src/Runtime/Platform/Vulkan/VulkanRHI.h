#pragma once
#include "Runtime/Function/RHI/RHI.h"
#include "VulkanViewport.h"
#include "VulkanDevice.h"
#include "VulkanFrameResource.h"
#include "Runtime/Core/Base/Singleton.h"

#include "vk_mem_alloc.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
namespace rhi {
    class VulkanRHI : public RHI
    {
    public:
        virtual void Init() override;
        virtual void Shutdown() override;

        // @brief use static_cast<VkInstance>(...) to get the actual instance
        virtual void* GetNativeInstance() override;
        // @brief use static_cast<VkDevice>(...) to get the actual logical device
        virtual void* GetNativeDevice() override;
        // @brief use static_cast<VkPhysicalDevice>(...) to get the actual physical device
        virtual void* GetNativePhysicalDevice() override;
        // @brief use static_cast<VkQueue>(...) to get the actual queue
        virtual void* GetNativeGraphicsQueue() override;
        // @brief use static_cast<VkQueue>(...) to get the actual queue
        virtual void* GetNativeComputeQueue() override;

        virtual void* GetCurrentFrame() override;
        virtual uint32_t GetGfxQueueFamily() override;

        virtual void Begin() override;
        virtual void BeginRenderPass(renderer::RenderPass& pass) override;
        virtual void SetViewport( float x, float y, float width, float height, float min_depth, float max_depth) override;
        virtual void SetScissor(int32_t offset_x, int32_t offset_y, uint32_t width, uint32_t height) override;

        virtual void GfxQueueSubmit() override;
        virtual void ComputeQueueSubmit() override;
        virtual void TransferQueueSubmit()override;
        virtual void EndRenderPass() override;
        virtual void End() override;

        virtual std::shared_ptr<RHITexture2D> RHICreateTexture2D(uint32_t width, uint32_t height, PixelFormat in_format, uint32_t miplevels = 0) override;
        virtual std::shared_ptr<RHITexture2D> RHICreateTexture2D(std::string_view path, uint32_t miplevels = 0) override;
        virtual renderer::RenderPass*         RHICreateRenderPass(const char* render_pass_name, const renderer::RenderPassDesc& desc,
            const std::function<void(renderer::RenderPass*)>& setup,
            const std::function<void(renderer::RenderPass*)>& exec) override;

        virtual void ImGui_ImplMLE_RenderDrawData(ImDrawData* draw_data) override;

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

        VulkanFrameResourceManager* frame_manager_;
    };
}