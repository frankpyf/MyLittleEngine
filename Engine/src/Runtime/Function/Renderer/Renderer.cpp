#include "mlepch.h"
#include "Renderer.h"
#include "RenderPass.h"
#include "Runtime/Function/RHI/RHI.h"
#include "Runtime/Function/RHI/RHIResource.h"
#include "Runtime/Function/RHI/RHICommands.h"

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
        auto& rhi = rhi::RHI::GetRHIInstance();
        auto& current_frame = frames_manager_.BeginFrame();

        rhi.AcquireNextImage(current_frame.command_buffer_->GetImageAvailableSemaphore());

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
        rhi::RHICommands::GfxQueueSubmit(current_frame.command_buffer_);
        rhi::RHICommands::Present(current_frame.command_buffer_->GetRenderFinishedSemaphore());
    }
}