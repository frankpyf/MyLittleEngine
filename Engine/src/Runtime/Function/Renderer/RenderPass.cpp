#include "mlepch.h"
#include "RenderPass.h"
#include "Runtime/Platform/Vulkan/VulkanRenderPass.h"
#include "Runtime/Function/RHI/RHI.h"

namespace renderer {
	RenderPass* RenderPass::Create(const char* render_pass_name, const RenderPassDesc& desc,
		const std::function<void(RenderPass*)>& setup,
		const std::function<void(RenderPass*)>& exec)
	{
		rhi::RHI& rhi = rhi::RHI::GetRHIInstance();
		return rhi.RHICreateRenderPass(render_pass_name, desc, setup, exec);
	}

	void RenderGraph::AddRenderPass(const char* render_pass_name, const RenderPassDesc& desc,
		const std::function<void(RenderPass*)>& setup,
		const std::function<void(RenderPass*)>& exec)
	{
		auto rp = RenderPass::Create(render_pass_name, desc, setup, exec);
		render_passes_.emplace_back(rp);
		
		is_compiled_ = false;
	}

	void RenderGraph::RemoveRenderPass(const char* render_pass_name)
	{
		render_passes_.remove_if([render_pass_name](auto rp)->bool
			{
				return rp->GetName() == render_pass_name;
			});

		is_compiled_ = false;
		render_pass_path_.clear();
	}

	void RenderGraph::Clear()
	{
		for (auto rp : render_passes_)
		{
			delete rp;
		}
		render_passes_.clear();
		render_pass_path_.clear();
	}

	void RenderGraph::Compile()
	{
		// temp
		Setup();
		
		is_compiled_ = true;
	}

	void RenderGraph::Setup()
	{
		for (auto pass:render_passes_)
		{
			pass->Setup();
		}
	}

	void RenderGraph::Run()
	{
		if (!is_compiled_)
			Compile();
		// TEMP!!!!!!!!!!!!!!!!!!
		for (auto pass:render_passes_)
		{
			pass->Exec();
		}
	}
}