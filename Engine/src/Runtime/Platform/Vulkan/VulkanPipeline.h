#pragma once
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
#include "Runtime/Function/Renderer/Pipeline.h"
namespace rhi {
	class VulkanRHI;
}
namespace renderer {
	class VulkanPipeline : public Pipeline
	{
	public:
		VulkanPipeline(rhi::VulkanRHI& in_rhi,
					   const char* vert_path,
					   const char* frag_path,
					   const PipelineDesc& desc);
		virtual ~VulkanPipeline();

		void DestroyPipeline();
		
		VulkanPipeline(const VulkanPipeline&) = delete;
		void operator = (const VulkanPipeline&) = delete;

		virtual void* GetHandle() override { return (void*)pipeline_; };
	private:
		std::vector<char> ReadFile(const char* file_path);

		void CreateGraphicsPipeline(const PipelineDesc& desc);
		void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shader_module);
	protected:
		VkPipeline  pipeline_;

		// Temp
		VkPipelineLayout pipeline_layout_;

		VkShaderModule vert_shader_module_;
		VkShaderModule frag_shader_module_;

		rhi::VulkanRHI& rhi_;
	};
}
