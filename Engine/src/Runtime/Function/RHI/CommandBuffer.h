#pragma once
struct ImDrawData;

namespace renderer {
    class RenderTarget;
    class RenderPass;
    class Pipeline;
}

namespace rhi {
    class RHIGraphicsEncoder
	{
	public:
        virtual ~RHIGraphicsEncoder() = default;

        
        virtual void BeginRenderPass(renderer::RenderPass& pass,
                                     renderer::RenderTarget& render_target) = 0;
        virtual void BindGfxPipeline(renderer::Pipeline* pipeline) = 0;
        virtual void SetViewport(float x, float y, float width, float height, float min_depth, float max_depth) = 0;
        virtual void SetScissor(int32_t offset_x, int32_t offset_y, uint32_t width, uint32_t height) = 0;
        virtual void Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) = 0;

        virtual void NextSubpass() {};

        virtual void EndRenderPass() = 0;

        virtual void ImGui_RenderDrawData(ImDrawData* draw_data) = 0;
	};

    // TODO: Add more virtual functions
    class RHIComputeEncoder
    {
    public:
        virtual ~RHIComputeEncoder() = default;
        virtual void BeginComputePass() = 0;
        virtual void EndComputePass() = 0;
    };

    class CommandBuffer
    {
    public:
        virtual ~CommandBuffer() = default;

        virtual void AllocateCommandBuffers() {};
        virtual void Begin() {};
        virtual void End() {};
        virtual RHIGraphicsEncoder& GetGfxEncoder() = 0;
        virtual void* GetNativeGfxHandle()          = 0;
        virtual void* GetImageAvailableSemaphore()  = 0;
        virtual void* GetRenderFinishedSemaphore()  = 0;
        virtual void* GetInFlightFence()            = 0;

        static CommandBuffer* Create();
    };
}