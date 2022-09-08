#pragma once
#include <vulkan/vulkan.h>
namespace engine {
	struct PipelineConfigInfo {
		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineInputAssemblyStateCreateInfo input_assembly_info;
		VkPipelineRasterizationStateCreateInfo rasterization_info;
		VkPipelineMultisampleStateCreateInfo multi_sample_info;
		VkPipelineColorBlendAttachmentState color_blend_attachment;
		VkPipelineColorBlendStateCreateInfo color_blend_info;
		VkPipelineDepthStencilStateCreateInfo depth_stencil_info;
		VkPipelineLayout pipeline_layout = nullptr;
		VkRenderPass render_pass = nullptr;
		uint32_t subpass = 0;
	};

	class VulkanPipeline
	{
	public:
		friend class VulkanDevice;
		VulkanPipeline(VulkanDevice* in_device,
			const std::string vert_path,
			const std::string frag_path,
			const PipelineConfigInfo& config);
		virtual ~VulkanPipeline();
		
		VulkanPipeline(const VulkanPipeline&) = delete;
		void operator = (const VulkanPipeline&) = delete;

		static PipelineConfigInfo DefaultPipelineConfigInfo(uint32_t width, uint32_t height);

		void Bind(VkCommandBuffer command_buffer)
		{
			vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_);
		}
	private:
		std::vector<char> ReadFile(const std::string& filename);

		void CreateGraphicsPipeline(
			const std::string vert_path,
			const std::string frag_path,
			const PipelineConfigInfo& config
		);

		void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shader_module);
	protected:
		VulkanDevice* device_;
		VkPipeline  pipeline_;
		VkShaderModule vert_shader_module_;
		VkShaderModule frag_shader_module_;
		
	};
}
