#include "mlepch.h"
#include "VulkanPipeline.h"
#include "VulkanDevice.h"
#include "Runtime/Core/Base/Log.h"

void check_vk_result(VkResult err)
{
	if (err == 0)
		return;
	MLE_CORE_ERROR("[VulkanPipeline] Error: VkResult = {0}\n", err);
	// fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
	if (err < 0)
		abort();
}

namespace engine {
	VulkanPipeline::VulkanPipeline(VulkanDevice* in_device,
		const std::string vert_path,
		const std::string frag_path,
		const PipelineConfigInfo& config)
		:device_(in_device)
	{
		CreateGraphicsPipeline(vert_path, frag_path, config);
	}

	VulkanPipeline::~VulkanPipeline()
	{
		vkDestroyShaderModule(device_->GetDeviceHandle(), vert_shader_module_, nullptr);
		vkDestroyShaderModule(device_->GetDeviceHandle(), frag_shader_module_, nullptr);
		vkDestroyPipeline(device_->GetDeviceHandle(), pipeline_, nullptr);
	}

	std::vector<char> VulkanPipeline::ReadFile(const std::string& filename) 
	{
		std::ifstream file(filename, std::ios::ate | std::ios::binary);

		if (!file.is_open()) {
			throw std::runtime_error("failed to open file!");
		}

		size_t fileSize = (size_t)file.tellg();
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize);

		file.close();

		return buffer;
	}

	void VulkanPipeline::CreateGraphicsPipeline(
		const std::string vert_path,
		const std::string frag_path,
		const PipelineConfigInfo& config
	)
	{
		auto vert_shader_code = ReadFile("shaders/vert.spv");
		auto frag_shader_code = ReadFile("shaders/frag.spv");

		CreateShaderModule(vert_shader_code, &vert_shader_module_);
		CreateShaderModule(frag_shader_code, &frag_shader_module_);

		VkPipelineShaderStageCreateInfo shader_stages[2];
		shader_stages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shader_stages[0].module = vert_shader_module_;
		shader_stages[0].pName = "main";
		shader_stages[0].flags = 0;
		shader_stages[0].pNext = nullptr;
		shader_stages[0].pSpecializationInfo = nullptr;
		shader_stages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shader_stages[1].module = frag_shader_module_;
		shader_stages[1].pName = "main";
		shader_stages[1].flags = 0;
		shader_stages[1].pNext = nullptr;
		shader_stages[1].pSpecializationInfo = nullptr;

		VkPipelineVertexInputStateCreateInfo vertex_input_info{};
		vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_info.vertexAttributeDescriptionCount = 0;
		vertex_input_info.vertexBindingDescriptionCount = 0;
		vertex_input_info.pVertexAttributeDescriptions = nullptr;
		vertex_input_info.pVertexBindingDescriptions = nullptr;

		VkPipelineViewportStateCreateInfo viewport_info{};
		viewport_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_info.viewportCount = 1;
		viewport_info.pViewports = &config.viewport;
		viewport_info.scissorCount = 1;
		viewport_info.pScissors = &config.scissor;

		VkGraphicsPipelineCreateInfo pipeline_info{};
		pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipeline_info.stageCount = 2;
		pipeline_info.pStages = shader_stages;
		pipeline_info.pVertexInputState = &vertex_input_info;
		pipeline_info.pInputAssemblyState = &config.input_assembly_info;
		pipeline_info.pViewportState = &viewport_info;
		pipeline_info.pRasterizationState = &config.rasterization_info;
		pipeline_info.pMultisampleState = &config.multi_sample_info;
		pipeline_info.pColorBlendState = &config.color_blend_info;
		pipeline_info.pDepthStencilState = &config.depth_stencil_info;
		pipeline_info.pDynamicState = nullptr;

		pipeline_info.layout = config.pipeline_layout;
		pipeline_info.renderPass = config.render_pass;
		pipeline_info.subpass = config.subpass;

		pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
		pipeline_info.basePipelineIndex = -1;

		VkResult result = vkCreateGraphicsPipelines(device_->GetDeviceHandle(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline_);
		check_vk_result(result);
	}

	void VulkanPipeline::CreateShaderModule(const std::vector<char>& code, VkShaderModule* shader_module)
	{
		VkShaderModuleCreateInfo shader_module_create_info{};
		shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_module_create_info.codeSize = code.size();
		shader_module_create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkResult result;
		result = vkCreateShaderModule(device_->GetDeviceHandle(), &shader_module_create_info, nullptr, shader_module);
		check_vk_result(result);
	}

	PipelineConfigInfo VulkanPipeline::DefaultPipelineConfigInfo(uint32_t width, uint32_t height)
	{
		PipelineConfigInfo config{};
		config.input_assembly_info.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		config.input_assembly_info.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		config.input_assembly_info.primitiveRestartEnable = VK_FALSE;
		
		config.viewport.x = 0.0f;
		config.viewport.y = 0.0f;
		config.viewport.width = static_cast<float>(width);
		config.viewport.height = static_cast<float>(height);
		config.viewport.minDepth = 0.0f;
		config.viewport.maxDepth = 1.0f;

		config.scissor.offset = { 0,0 };
		config.scissor.extent = { width,height };

		config.rasterization_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		config.rasterization_info.depthClampEnable = VK_FALSE;
		config.rasterization_info.rasterizerDiscardEnable = VK_FALSE;
		config.rasterization_info.polygonMode = VK_POLYGON_MODE_FILL;
		config.rasterization_info.lineWidth = 1.0f;
		config.rasterization_info.cullMode = VK_CULL_MODE_NONE;
		config.rasterization_info.frontFace - VK_FRONT_FACE_CLOCKWISE;
		config.rasterization_info.depthBiasEnable = VK_FALSE;
		config.rasterization_info.depthBiasConstantFactor = 0.0f;
		config.rasterization_info.depthBiasClamp = 0.0f;
		config.rasterization_info.depthBiasSlopeFactor = 0.0f;

		config.multi_sample_info.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		config.multi_sample_info.sampleShadingEnable = VK_FALSE;
		config.multi_sample_info.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		config.multi_sample_info.minSampleShading = 1.0f;
		config.multi_sample_info.pSampleMask = nullptr;
		config.multi_sample_info.alphaToCoverageEnable = VK_FALSE;
		config.multi_sample_info.alphaToOneEnable = VK_FALSE;

		config.color_blend_attachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		config.color_blend_attachment.blendEnable = VK_FALSE;
		config.color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		config.color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		config.color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
		config.color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		config.color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		config.color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;

		config.color_blend_info.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		config.color_blend_info.logicOpEnable = VK_FALSE;
		config.color_blend_info.logicOp = VK_LOGIC_OP_COPY;
		config.color_blend_info.attachmentCount = 1;
		config.color_blend_info.pAttachments = &config.color_blend_attachment;
		config.color_blend_info.blendConstants[0] = 0.0f;
		config.color_blend_info.blendConstants[1] = 0.0f;
		config.color_blend_info.blendConstants[2] = 0.0f;
		config.color_blend_info.blendConstants[3] = 0.0f;

		config.depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		config.depth_stencil_info.depthTestEnable = VK_TRUE;
		config.depth_stencil_info.depthWriteEnable = VK_TRUE;
		config.depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;
		config.depth_stencil_info.depthBoundsTestEnable = VK_FALSE;
		config.depth_stencil_info.minDepthBounds = 0.0f;
		config.depth_stencil_info.maxDepthBounds = 1.0f;
		config.depth_stencil_info.stencilTestEnable = VK_FALSE;
		config.depth_stencil_info.front = {};
		config.depth_stencil_info.back = {};

		return config;
	}
}