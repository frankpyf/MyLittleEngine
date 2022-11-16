#pragma once
#include "Runtime/Function/RHI/CommandBuffer.h"
#include "RenderPass.h"

namespace renderer {
    // Render Commands

    static void BeginRenderPass(rhi::CommandBuffer& cmd_buffer, RenderPass& pass, RenderTarget& render_target)
    {
        cmd_buffer.GetGfxEncoder().BeginRenderPass(pass, render_target);
    };
    static void BindGfxPipeline(rhi::CommandBuffer& cmd_buffer, renderer::Pipeline* pipeline)
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

    static void BindVertexBuffers(rhi::CommandBuffer& cmd_buffer, uint32_t first_binding, uint32_t binding_count, rhi::RHIVertexBuffer** buffer, uint64_t* offsets)
    {
        cmd_buffer.GetGfxEncoder().BindVertexBuffers(first_binding, binding_count, buffer, offsets);
    }
    // Transfer Commands
    static void CopyBufferToBuffer(rhi::CommandBuffer& cmd_buffer, const rhi::CopyBufferToBufferDesc& desc)
    {
        cmd_buffer.GetTransferEncoder().CopyBufferToBuffer(desc);
    }
}