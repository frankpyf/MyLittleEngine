#include "mlepch.h"
#include "FrameResource.h"
#include "RenderPass.h"
#include "Pipeline.h"
#include "Runtime/Platform/Vulkan/VulkanRenderPass.h"
#include "Runtime/Function/RHI/RHI.h"
#include "Runtime/Function/RHI/CommandBuffer.h"

namespace renderer {
	RenderPass::~RenderPass()
	{
		for (auto pipeline : pipelines_)
		{
			delete pipeline;
		}
		pipelines_.clear();

		MLE_CORE_INFO("{0} pass has been deconstructed", pass_name_);
	}

	RenderPass* RenderPass::Create(const char* render_pass_name, const RenderPassDesc& desc,
									EXEC_FUNC exec)
	{
		rhi::RHI& rhi = rhi::RHI::GetRHIInstance();
		return rhi.RHICreateRenderPass(render_pass_name, desc, exec);
	}

	Pipeline* RenderPass::CreatePipeline(const char* vert_path,
									     const char* frag_path,
										 const PipelineDesc& desc)
	{
		Pipeline* pipeline = Pipeline::Create(vert_path, frag_path, desc);
		pipelines_.emplace_back(pipeline);
		return pipeline;
	}

	RenderTarget* RenderTarget::Create(RenderPass& pass)
	{
		rhi::RHI& rhi = rhi::RHI::GetRHIInstance();
		switch (rhi::RHI::GetAPI())
		{
		case rhi::RHI::GfxAPI::None: return 0;
		case rhi::RHI::GfxAPI::Vulkan: return rhi.RHICreateRenderTarget(pass);
		}
	}

	RenderPass& RenderGraph::GetRenderPass(const char* render_pass_name)
	{
		for (auto* pass : render_passes_)
		{
			if (pass->GetName() == render_pass_name)
			{
				return *pass;
			}
		}
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
		// TODO: topo sort
		for (auto rp : render_passes_)
		{
			render_pass_path_.emplace_back(rp);
		}
		is_compiled_ = true;
	}

	void RenderGraph::Run(FrameResource& resource)
	{
		if (!is_compiled_)
			Compile();
		// TEMP!!!!!!!!!!!!!!!!!!
		
		auto& cmd_buffer = resource.command_buffer;
		auto& rts = resource.render_targets;
		for (auto pass:render_passes_)
		{
			// TODO:Arena allocation
			// Create the framebuffer for this pass
			for (auto index : pass->GetRTDesc().attachments_index)
			{
				pass->GetRTDesc().attachments.emplace_back(resources_[index]->GetView());
				pass->GetRTDesc().width = resources_[index]->GetWidth();
				pass->GetRTDesc().height = resources_[index]->GetHeight();
			}

			if (pass->is_for_present_)
			{
				rhi::RHI& rhi = rhi::RHI::GetRHIInstance();
				pass->GetRTDesc().attachments.emplace_back(rhi.GetNativeSwapchainImageView());
				pass->GetRTDesc().width = rhi.GetViewportWidth();
				pass->GetRTDesc().height = rhi.GetViewportHeight();
			}
			RenderTarget* rt = RenderTarget::Create(*pass);
			pass->exec_func_(*pass,*rt, *cmd_buffer);
			pass->GetRTDesc().attachments.clear();
			rts.emplace_back(rt);
		}
	}

	void RenderGraph::PostRun()
	{
		
	}

	uint32_t RenderGraph::RegisterResource(rhi::RHITexture2D* resource)
	{
		resources_.emplace_back(resource);
		return resources_.size() - 1;
	}

	uint32_t RenderGraph::RegisterTransientResource()
	{
		rhi::RHITexture2D* resource;
		resources_.emplace_back(resource);
		return resources_.size() - 1;
	}
}