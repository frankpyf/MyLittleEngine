#pragma once
#include "Runtime/Function/RHI/CommandBuffer.h"
#include "Runtime/Function/RHI/RenderPass.h"
#include "Runtime/Function/RHI/Descriptor.h"

namespace renderer {
    // Render Commands

    static void BeginRenderPass(rhi::CommandBuffer& cmd_buffer, rhi::RenderPass& pass, rhi::RenderTarget& render_target)
    {
        cmd_buffer.GetGfxEncoder().BeginRenderPass(pass, render_target);
    };
    static void BindGfxPipeline(rhi::CommandBuffer& cmd_buffer, rhi::RHIPipeline* pipeline)
    {
        cmd_buffer.GetGfxEncoder().BindGfxPipeline(pipeline);
    };
    static void SetViewport(rhi::CommandBuffer& cmd_buffer, float x, float y, float width, float height, float min_depth = 0.0f, float max_depth = 1.0f)
    {
        cmd_buffer.GetGfxEncoder().SetViewport(x, y, width, height, min_depth, max_depth);
    };
    static void SetScissor(rhi::CommandBuffer& cmd_buffer, int32_t offset_x, int32_t offset_y, uint32_t width, uint32_t height)
    {
        cmd_buffer.GetGfxEncoder().SetScissor(offset_x, offset_y, width, height);
    };
    static void Draw(rhi::CommandBuffer& cmd_buffer, uint32_t vertex_count, uint32_t instance_count, uint32_t first_vertex = 0, uint32_t first_instance = 0)
    {
        cmd_buffer.GetGfxEncoder().Draw(vertex_count, instance_count, first_vertex, first_instance);
    }
    static void DrawIndexed(rhi::CommandBuffer& cmd_buffer, uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t offset, uint32_t first_instance)
    {
        cmd_buffer.GetGfxEncoder().DrawIndexed(index_count, instance_count, first_index, offset, first_instance);
    }
    static void NextSubpass(rhi::CommandBuffer& cmd_buffer)
    {
        cmd_buffer.GetGfxEncoder().NextSubpass();
    }
    static void EndRenderPass(rhi::CommandBuffer& cmd_buffer)
    {
        cmd_buffer.GetGfxEncoder().EndRenderPass();
    }
    static void ImGui_RenderDrawData(rhi::CommandBuffer& cmd_buffer, ImDrawData* draw_data)
    {
        cmd_buffer.GetGfxEncoder().ImGui_RenderDrawData(draw_data);
    }

    static void BindVertexBuffers(rhi::CommandBuffer& cmd_buffer, uint32_t first_binding, uint32_t binding_count, rhi::RHIBuffer** buffer, uint64_t* offsets)
    {
        cmd_buffer.GetGfxEncoder().BindVertexBuffers(first_binding, binding_count, buffer, offsets);
    }
    static void BindIndexBuffer(rhi::CommandBuffer& cmd_buffer, rhi::RHIBuffer* index_buffer, uint64_t offset)
    {
        cmd_buffer.GetGfxEncoder().BindIndexBuffer(index_buffer, offset);
    }
    static void BindDescriptorSets(rhi::CommandBuffer& cmd_buffer, rhi::PipelineLayout* layout, uint32_t first_set, uint32_t sets_count, rhi::DescriptorSet** sets, uint32_t dynameic_offset_count, const uint32_t* dynamic_offsets)
    {
        cmd_buffer.GetGfxEncoder().BindDescriptorSets(layout, first_set, sets_count, sets, dynameic_offset_count, dynamic_offsets);
    }
    // Transfer Commands
    static void CopyBufferToBuffer(rhi::CommandBuffer& cmd_buffer, const rhi::CopyBufferToBufferDesc& desc)
    {
        cmd_buffer.GetTransferEncoder().CopyBufferToBuffer(desc);
    }
}