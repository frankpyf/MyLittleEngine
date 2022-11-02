#pragma once
#include "RHIResource.h"
#include "Runtime/Function/Renderer/RenderPass.h"
#include "Runtime/Function/Renderer/Pipeline.h"
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

        virtual void* GetNativeSwapchainImageView() = 0;
        virtual uint32_t GetViewportWidth() = 0;
        virtual uint32_t GetViewportHeight() = 0;

        virtual void* GetCurrentFrame() = 0;
        virtual uint32_t GetGfxQueueFamily() = 0;

        virtual void Begin() = 0;
        virtual void BeginRenderPass(renderer::RenderPass& pass,
                                     renderer::RenderTarget& render_target) = 0;
        virtual void BindGfxPipeline(renderer::Pipeline* pipeline) = 0;
        virtual void SetViewport(float x, float y, float width, float height, float min_depth, float max_depth) = 0;
        virtual void SetScissor(int32_t offset_x, int32_t offset_y, uint32_t width, uint32_t height) = 0;
        virtual void Draw(uint32_t vertex_count, uint32_t instance_count) = 0;

        virtual void NextSubpass() {};
        virtual void GfxQueueSubmit() = 0;
        virtual void ComputeQueueSubmit() = 0;
        virtual void TransferQueueSubmit() = 0;
        virtual void EndRenderPass() = 0;
        virtual void End() = 0;

        // TODO: maybe come up with a better solution
        virtual void ImGui_ImplMLE_RenderDrawData(ImDrawData* draw_data) = 0;

        virtual std::shared_ptr<RHITexture2D> RHICreateTexture2D(uint32_t width, uint32_t height, PixelFormat in_format, uint32_t miplevels = 1) = 0;
        virtual std::shared_ptr<RHITexture2D> RHICreateTexture2D(std::string_view path, uint32_t miplevels = 1) = 0;
        virtual renderer::RenderPass* RHICreateRenderPass(const char* render_pass_name, const renderer::RenderPassDesc& desc,
                                                          void(*exec)(renderer::RenderPass& rp, renderer::RenderTarget& rt)) = 0;
        virtual renderer::RenderTarget* RHICreateRenderTarget(renderer::RenderPass& pass) = 0;
        virtual renderer::Pipeline* RHICreatePipeline(const char* vert_path,
                                                      const char* frag_path,
                                                      const renderer::PipelineDesc& desc) = 0;
        static GfxAPI GetAPI() { return api_; }
        static RHI& GetRHIInstance();
    private:
        static GfxAPI api_;
    };
} // namespace engine
