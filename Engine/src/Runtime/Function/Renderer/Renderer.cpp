#include "mlepch.h"
#include "Renderer.h"
#include "RenderPass.h"
#include "Runtime/Function/RHI/RHICommands.h"
#include "Runtime/Function/RHI/RHIResource.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

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
    }

    void Renderer::Shutdown()
    {
        rhi::RHICommands::Shutdown();
    }

  

    void Renderer::Begin()
    {
        rhi::RHICommands::Begin();
    }

    void Renderer::Tick(float time_step)
    {
        renderer::RenderGraph& render_graph = renderer::RenderGraph::GetInstance();
        render_graph.Run();
    }

    void Renderer::End()
    {
        rhi::RHICommands::End();
        renderer::RenderGraph& render_graph = renderer::RenderGraph::GetInstance();
        render_graph.PostRun();
    }
}