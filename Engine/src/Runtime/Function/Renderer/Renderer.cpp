#include "mlepch.h"
#include "Renderer.h"
#include "RenderPass.h"
#include "Runtime/Function/RHI/RHI.h"
#include "Runtime/Function/RHI/RHIResource.h"
#include "Runtime/Function/RHI/RHICommands.h"
#include "Runtime/Function/Renderer/RenderCommands.h"
#include "Runtime/Resource/Vertex.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "backends/imgui_impl_glfw.h"

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

    }

    void Renderer::Tick(float time_step)
    {
        auto& current_frame = frames_manager_.GetCurrentFrame();
        renderer::RenderGraph& render_graph = renderer::RenderGraph::GetInstance();
        render_graph.Run(current_frame);
    }

    void Renderer::End()
    {
        auto& current_frame = frames_manager_.GetCurrentFrame();
        frames_manager_.EndFrame();
        renderer::RenderGraph& render_graph = renderer::RenderGraph::GetInstance();
        render_graph.PostRun();

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

    std::shared_ptr<rhi::RHIVertexBuffer> Renderer::LoadModel(const std::vector<resource::Vertex> in_vertices)
    {
        rhi::RHI& rhi = rhi::RHI::GetRHIInstance();
        auto size = sizeof(in_vertices[0]) * in_vertices.size();
        auto vertex_buffer = rhi.RHICreateVertexBuffer(size);
        auto staging_buffer = rhi.RHICreateStagingBuffer(size);
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
}