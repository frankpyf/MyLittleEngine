#pragma once
#include <vulkan/vulkan.h>
#include "VulkanPipeline.h"
#include "Runtime/Function/Renderer/RenderPass.h"
namespace rhi {
	class VulkanRHI;
}

namespace renderer {
	struct VulkanRenderPipeline
	{
		rhi::VulkanPipeline pipeline;
		VkPipelineLayout layout;
	};

	class VulkanRenderPass : public RenderPass
	{
	public:
		VulkanRenderPass(rhi::VulkanRHI& in_rhi, const char* render_pass_name, const RenderPassDesc& desc,
			const std::function<void(RenderPass*)>& setup,
			const std::function<void(RenderPass*)>& exec);
		virtual ~VulkanRenderPass();

		virtual void* GetHandle() override { return (void*)render_pass_; };
		virtual void* GetFramebuffer(uint32_t index) { return (void*)framebuffer_[index]; };
		virtual uint32_t GetWidth() override { return width_; };
		virtual uint32_t GetHeight() override { return height_; };
	private:
		void CreateRenderPass(RenderPassDesc desc);
		// Setup Internal dependency
		void SetupDependency(uint32_t src_index, uint32_t dst_index);

		void SetupIMGUI(uint32_t final_subpass_index);

		VkRenderPass render_pass_ = VK_NULL_HANDLE;

		std::vector<VkFramebuffer> framebuffer_{};

		uint32_t width_ = 0;
		uint32_t height_ = 0;
		
		rhi::VulkanRHI& rhi_;
	};
}

