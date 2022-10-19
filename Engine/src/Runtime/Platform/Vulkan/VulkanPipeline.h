#pragma once
#include <vulkan/vulkan.h>
namespace rhi {
	struct PipelineConfigInfo {
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
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

		void DestroyPipeline();
		
		VulkanPipeline(const VulkanPipeline&) = delete;
		void operator = (const VulkanPipeline&) = delete;

		static void DefaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

		VkPipeline& GetPipeline() { return pipeline_; };
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
