#pragma once
#include <vulkan/vulkan.h>
#include "Runtime/Function/RHI/RenderPass.h"
namespace rhi {
	class VulkanRHI;

	class VulkanRenderTarget :public RenderTarget
	{
	public:
		VulkanRenderTarget(rhi::VulkanRHI& in_rhi, const RenderTarget::Descriptor& desc);
		virtual ~VulkanRenderTarget();
		virtual void* GetHandle() override { return (void*)framebuffer_; };
	private:
		void DestroyFramebuffer();
		void CreateFramebuffer(const RenderTarget::Descriptor& desc);

		VkFramebuffer framebuffer_ = VK_NULL_HANDLE;

		rhi::VulkanRHI& rhi_;
	};

	class VulkanRenderPass : public RenderPass
	{
	public:
		VulkanRenderPass(rhi::VulkanRHI& in_rhi, const RenderPass::Descriptor& desc);
		virtual ~VulkanRenderPass();

		virtual void* GetHandle() override { return (void*)render_pass_; };
		
	private:
		void CreateRenderPass(RenderPass::Descriptor desc);
		// Setup Internal dependency
		void SetupDependency(uint32_t src_index, uint32_t dst_index);

		VkRenderPass render_pass_ = VK_NULL_HANDLE;
		
		rhi::VulkanRHI& rhi_;
	};
}

