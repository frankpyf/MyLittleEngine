#pragma once
#include "RHIResource.h"
#include "Runtime/Function/Renderer/RenderPass.h"
// Forward declaration
struct ImDrawData;

namespace rhi{

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

        // @brief use static_cast<...> to get the actual instance
        virtual void* GetNativeInstance() = 0;
        // @brief use static_cast<...> to get the actual logical device
        virtual void* GetNativeDevice() = 0;
        // @brief use static_cast<...> to get the actual physical device
        virtual void* GetNativePhysicalDevice() = 0;
        // @brief use static_cast<...> to get the actual queue
        virtual void* GetNativeGraphicsQueue() = 0;
        // @brief use static_cast<...> to get the actual queue
        virtual void* GetNativeComputeQueue() = 0;

        virtual void* GetCurrentFrame() = 0;
        virtual uint32_t GetGfxQueueFamily() = 0;

        virtual void Begin() = 0;
        virtual void BeginRenderPass(renderer::RenderPass& pass) = 0;
        virtual void SetViewport(float x, float y, float width, float height, float min_depth, float max_depth) = 0;
        virtual void SetScissor(int32_t offset_x, int32_t offset_y, uint32_t width, uint32_t height) = 0;

        virtual void GfxQueueSubmit() = 0;
        virtual void ComputeQueueSubmit() = 0;
        virtual void TransferQueueSubmit() = 0;
        virtual void EndRenderPass() = 0;
        virtual void End() = 0;

        // TODO: maybe come up with a better solution
        virtual void ImGui_ImplMLE_RenderDrawData(ImDrawData* draw_data) = 0;

        virtual std::shared_ptr<RHITexture2D> RHICreateTexture2D(uint32_t width, uint32_t height, PixelFormat in_format, uint32_t miplevels = 0) = 0;
        virtual std::shared_ptr<RHITexture2D> RHICreateTexture2D(std::string_view path, uint32_t miplevels = 0) = 0;
        virtual renderer::RenderPass* RHICreateRenderPass(const char* render_pass_name, const renderer::RenderPassDesc& desc,
                                                                          const std::function<void(renderer::RenderPass*)>& setup,
                                                                          const std::function<void(renderer::RenderPass*)>& exec) = 0;
        static GfxAPI GetAPI() { return api_; }
        static RHI& GetRHIInstance();
    private:
        static GfxAPI api_;
    };
} // namespace engine
