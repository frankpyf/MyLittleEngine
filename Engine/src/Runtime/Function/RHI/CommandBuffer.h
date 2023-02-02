#pragma once
#include "Descriptor.h"
#include "RHIResource.h"
struct ImDrawData;

namespace rhi {
    class RenderTarget;
    class RenderPass;


    struct CopyBufferToBufferDesc
    {
        RHIBuffer* src;
        uint64_t src_offset;
        RHIBuffer* dst;
        uint64_t dst_offset;
        uint64_t size;
    };
    class RHIEncoderBase
    {
    public:
        virtual void Begin() {};

        virtual void End() {};

        virtual void* GetHandle() = 0;
    };

    class RHIGraphicsEncoder : public RHIEncoderBase
	{
	public:
        virtual ~RHIGraphicsEncoder() = default;

        
        virtual void BeginRenderPass(RenderPass& pass, RenderTarget& render_target) = 0;
        virtual void BindGfxPipeline(RHIPipeline* pipeline) = 0;
        virtual void BindVertexBuffers(uint32_t first_binding, uint32_t binding_count, RHIBuffer** buffer, uint64_t* offsets) = 0;
        virtual void BindIndexBuffer(RHIBuffer* index_buffer, uint64_t offset) = 0;
        virtual void BindDescriptorSets(PipelineLayout* layout, uint32_t first_set, uint32_t sets_count, DescriptorSet** sets, uint32_t dynameic_offset_count, const uint32_t* dynamic_offsets) = 0;
        
        virtual void SetViewport(float x, float y, float width, float height, float min_depth, float max_depth) = 0;
        virtual void SetScissor(int32_t offset_x, int32_t offset_y, uint32_t width, uint32_t height) = 0;

        virtual void Draw(uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex, uint32_t first_instance) = 0;
        virtual void DrawIndexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t offset, uint32_t first_instance) = 0;

        virtual void NextSubpass() {};

        virtual void EndRenderPass() = 0;

        virtual void ImGui_RenderDrawData(ImDrawData* draw_data) = 0;
	};

    class RHIComputeEncoder : public RHIEncoderBase
    {
    public:
        virtual ~RHIComputeEncoder() = default;
        virtual void BeginComputePass() = 0;
        virtual void EndComputePass() = 0;
    };

    class RHITransferEncoder : public RHIEncoderBase
    {
    public:
        virtual ~RHITransferEncoder() = default;
        
        virtual void CopyBufferToBuffer(const CopyBufferToBufferDesc& desc)   {};
        virtual void CopyBufferToImage(RHIBuffer*            buffer,
                                       RHITexture*           image,
                                       uint32_t              width,
                                       uint32_t              height,
                                       uint32_t              layer_count)    {};
    };

    class CommandBuffer
    {
    public:
        virtual ~CommandBuffer() = default;

        virtual void AllocateCommandBuffers() {};
        virtual void Begin() {};
        virtual void End() {};
        virtual RHIGraphicsEncoder& GetGfxEncoder()         = 0;
        virtual void* GetNativeGfxHandle()                  = 0;
        virtual RHITransferEncoder& GetTransferEncoder()    = 0;
        virtual void* GetNativeTransferHandle()             = 0;
    };
}