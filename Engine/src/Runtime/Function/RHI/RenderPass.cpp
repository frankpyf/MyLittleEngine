#include "mlepch.h"
#include "Runtime/Function/Renderer/FrameResource.h"
#include "RenderPass.h"
#include "RHI.h"
#include "CommandBuffer.h"
#include "Runtime/Platform/Vulkan/VulkanRenderPass.h"

namespace rhi {
	RenderPass::~RenderPass()
	{
		pipelines_.clear();
	}

	std::unique_ptr<RenderPass> RenderPass::Create(const Descriptor& desc)
	{
		RHI& rhi = RHI::GetRHIInstance();
		return rhi.RHICreateRenderPass(desc);
	}

	PipelineRef RenderPass::CreatePipeline(const RHIPipeline::Descriptor& desc)
	{
		rhi::RHI& rhi = rhi::RHI::GetRHIInstance();

		PipelineRef pipeline = rhi.RHICreatePipeline(desc);
		pipelines_.emplace_back(pipeline);
		return pipeline;
	}

	std::unique_ptr<RenderTarget> RenderTarget::Create(const RenderTarget::Descriptor& desc)
	{
		rhi::RHI& rhi = rhi::RHI::GetRHIInstance();
		return rhi.RHICreateRenderTarget(desc);
	}
}