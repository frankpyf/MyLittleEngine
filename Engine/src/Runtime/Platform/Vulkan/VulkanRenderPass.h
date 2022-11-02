#pragma once
#include <vulkan/vulkan.h>
#include "Runtime/Function/Renderer/RenderPass.h"
namespace rhi {
	class VulkanRHI;
}

namespace renderer {
	class VulkanRenderTarget :public RenderTarget
	{
	public:
		VulkanRenderTarget(rhi::VulkanRHI& in_rhi, RenderPass& pass);
		virtual ~VulkanRenderTarget();
		virtual void* GetHandle() override { return (void*)framebuffer_; };
	private:
		void DestroyFramebuffer();
		void CreateFramebuffer(RenderPass& pass);

		VkFramebuffer framebuffer_ = VK_NULL_HANDLE;

		rhi::VulkanRHI& rhi_;
	};

	class VulkanRenderPass : public RenderPass
	{
	public:
		VulkanRenderPass(rhi::VulkanRHI& in_rhi, const char* render_pass_name, const RenderPassDesc& desc,
			void(*exec)(RenderPass& rp, RenderTarget& rt));
		virtual ~VulkanRenderPass();

		virtual void* GetHandle() override { return (void*)render_pass_; };
		
	private:
		void CreateRenderPass(RenderPassDesc desc);
		// Setup Internal dependency
		void SetupDependency(uint32_t src_index, uint32_t dst_index);

		void SetupIMGUI(uint32_t present_subpass_index);

		VkRenderPass render_pass_ = VK_NULL_HANDLE;
		
		rhi::VulkanRHI& rhi_;
	};
}

