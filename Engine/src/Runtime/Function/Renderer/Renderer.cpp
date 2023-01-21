#include "mlepch.h"
#include "Renderer.h"
#include "Runtime/Function/RHI/RenderPass.h"
#include "Runtime/Function/RHI/RHI.h"
#include "Runtime/Function/RHI/RHIResource.h"
#include "Runtime/Function/RHI/RHICommands.h"
#include "Runtime/Function/Renderer/RenderCommands.h"
#include "Runtime/Resource/Vertex.h"
#include "RenderGraph/RenderGraph.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "backends/imgui_impl_glfw.h"

#include "ImGuizmo.h"

namespace renderer
{
    Renderer::Renderer()
    {
    }

    Renderer::~Renderer()
    {
    }

    void Renderer::Init()
    {
        rhi::RHICommands::Init();
        frames_manager_.CreateFrames();
    }

    void Renderer::Shutdown()
    {
        frames_manager_.DestroyFrames();
        rhi::RHICommands::Shutdown();
    }

    void Renderer::Begin()
    {
        frames_manager_.BeginFrame();

        // Start the Dear ImGui frame
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();
    }

    void Renderer::Tick(float time_step)
    {
        auto& current_frame = frames_manager_.GetCurrentFrame();

        // Update Descriptors
        rhi::DescriptorWriter::Begin(desc_allocator_)
            .WriteBuffer(0, current_frame.global_ubo.get(), DESCRIPTOR_TYPE_UNIFORM_BUFFER)
            .Build(current_frame.global_set, global_layout_);

        render_graph_.Run(current_frame);
    }

    void Renderer::End()
    {
        auto& current_frame = frames_manager_.GetCurrentFrame();
        frames_manager_.EndFrame();

        rhi::QueueSubmitDesc gfx_submit_info{};
        gfx_submit_info.cmds_count = 1;
        rhi::RHIEncoderBase* encoders[] = { &current_frame.command_buffer->GetGfxEncoder() };
        gfx_submit_info.encoders = encoders;
        gfx_submit_info.signal_fence = current_frame.in_flight_fence;

        gfx_submit_info.signal_semaphore_count = 1;
        rhi::Semaphore* signal_semaphores[] = { current_frame.render_finished_semaphore };
        gfx_submit_info.signal_semaphore = signal_semaphores;

        gfx_submit_info.wait_semaphore_count = 1;
        rhi::Semaphore* wait_semaphores[] = { current_frame.image_acquired_semaphore };
        gfx_submit_info.wait_semaphore = wait_semaphores;

        rhi::RHICommands::GfxQueueSubmit(gfx_submit_info);
        rhi::Semaphore* present_semaphores[] = { current_frame.render_finished_semaphore };
        rhi::RHICommands::Present(present_semaphores, 1);
    }

    rhi::BufferRef Renderer::LoadModel(const std::vector<resource::Vertex>& in_vertices)
    {
        rhi::RHI& rhi = rhi::RHI::GetRHIInstance();
        auto size = sizeof(in_vertices[0]) * in_vertices.size();
        rhi::RHIBuffer::Descriptor vb_desc{}; 
        vb_desc.size = size;
        vb_desc.usage = ResourceTypes::RESOURCE_TYPE_VERTEX_BUFFER;
        vb_desc.memory_usage = MemoryUsage::MEMORY_USAGE_GPU_ONLY;
        vb_desc.prefer_device = true;
        auto vertex_buffer = rhi.RHICreateBuffer(vb_desc);

        rhi::RHIBuffer::Descriptor sb_desc{};
        sb_desc.size = size;
        sb_desc.usage = ResourceTypes::RESOURCE_TYPE_STAGING_BUFFER;
        sb_desc.memory_usage = MemoryUsage::MEMORY_USAGE_CPU_TO_GPU;
        sb_desc.prefer_host = true;
        sb_desc.mapped_at_creation = true;
        auto staging_buffer = rhi.RHICreateBuffer(sb_desc);
        staging_buffer->SetData(in_vertices.data(), size);

        auto& current_frame = frames_manager_.GetCurrentFrame();
        auto& cmd_buffer = current_frame.command_buffer->GetTransferEncoder();
        cmd_buffer.Begin();
        cmd_buffer.CopyBufferToBuffer({staging_buffer.get(), 0, vertex_buffer.get(), 0, size});
        cmd_buffer.End();

        rhi::QueueSubmitDesc submit_info{};
        rhi::RHIEncoderBase* encoders[] = { &cmd_buffer };
        submit_info.encoders = encoders;
        submit_info.cmds_count = 1;

        rhi::RHICommands::TransferQueueSubmit(submit_info);
        rhi.RHIBlockUntilGPUIdle();
        return vertex_buffer;
    }

    rhi::BufferRef Renderer::LoadIndex(const std::vector<uint16_t>& in_indecies)
    {
        rhi::RHI& rhi = rhi::RHI::GetRHIInstance();
        auto size = sizeof(in_indecies[0]) * in_indecies.size();
        rhi::RHIBuffer::Descriptor ib_desc{};
        ib_desc.size = size;
        ib_desc.usage = ResourceTypes::RESOURCE_TYPE_INDEX_BUFFER;
        ib_desc.memory_usage = MemoryUsage::MEMORY_USAGE_GPU_ONLY;
        ib_desc.prefer_device = true;
        auto index_buffer = rhi.RHICreateBuffer(ib_desc);

        rhi::RHIBuffer::Descriptor sb_desc{};
        sb_desc.size = size;
        sb_desc.usage = ResourceTypes::RESOURCE_TYPE_STAGING_BUFFER;
        sb_desc.memory_usage = MemoryUsage::MEMORY_USAGE_CPU_TO_GPU;
        sb_desc.prefer_host = true;
        sb_desc.mapped_at_creation = true;
        auto staging_buffer = rhi.RHICreateBuffer(sb_desc);
        staging_buffer->SetData(in_indecies.data(), size);

        auto& current_frame = frames_manager_.GetCurrentFrame();
        auto& cmd_buffer = current_frame.command_buffer->GetTransferEncoder();
        cmd_buffer.Begin();
        cmd_buffer.CopyBufferToBuffer({ staging_buffer.get(), 0, index_buffer.get(), 0, size });
        cmd_buffer.End();

        rhi::QueueSubmitDesc submit_info{};
        rhi::RHIEncoderBase* encoders[] = { &cmd_buffer };
        submit_info.encoders = encoders;
        submit_info.cmds_count = 1;

        rhi::RHICommands::TransferQueueSubmit(submit_info);
        rhi.RHIBlockUntilGPUIdle();
        return index_buffer;
    }
}