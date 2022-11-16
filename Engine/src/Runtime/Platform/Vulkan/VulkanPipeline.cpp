#include "mlepch.h"
#include "VulkanPipeline.h"
#include "VulkanRHI.h"
#include "Runtime/Function/Renderer/RenderPass.h"
#include "Runtime/Resource/Vertex.h"

namespace renderer {
	VulkanPipeline::VulkanPipeline(rhi::VulkanDevice* in_device,
								   const char* vert_path,
								   const char* frag_path,
								   const PipelineDesc& desc)
		:device_(in_device)
	{
		assert(
			desc.render_pass != nullptr &&
			"Cannot create graphics pipeline: no renderPass provided in configInfo");
		auto vertCode = ReadFile(vert_path);
		auto fragCode = ReadFile(frag_path);

		CreateShaderModule(vertCode, &vert_shader_module_);
		CreateShaderModule(fragCode, &frag_shader_module_);

		CreateGraphicsPipeline(desc);
	}

	VulkanPipeline::~VulkanPipeline()
	{
		DestroyPipeline();
	}

	void VulkanPipeline::DestroyPipeline()
	{
		vkDestroyShaderModule(device_->GetDeviceHandle(), vert_shader_module_, nullptr);
		vkDestroyShaderModule(device_->GetDeviceHandle(), frag_shader_module_, nullptr);
		vkDestroyPipelineLayout(device_->GetDeviceHandle(), pipeline_layout_, nullptr);
		vkDestroyPipeline(device_->GetDeviceHandle(), pipeline_, nullptr);
	}

	std::vector<char> VulkanPipeline::ReadFile(const char* file_path)
	{
		std::ifstream file(file_path, std::ios::ate | std::ios::binary);

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

	void VulkanPipeline::CreateGraphicsPipeline(const PipelineDesc& desc)
	{
		VkPipelineShaderStageCreateInfo shaderStages[2];
		shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStages[0].module = vert_shader_module_;
		shaderStages[0].pName = "main";
		shaderStages[0].flags = 0;
		shaderStages[0].pNext = nullptr;
		shaderStages[0].pSpecializationInfo = nullptr;

		shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStages[1].module = frag_shader_module_;
		shaderStages[1].pName = "main";
		shaderStages[1].flags = 0;
		shaderStages[1].pNext = nullptr;
		shaderStages[1].pSpecializationInfo = nullptr;

		
		// TEMP
		VkVertexInputBindingDescription binding_description{};
		binding_description.binding = 0;
		binding_description.stride = sizeof(resource::Vertex);
		binding_description.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputAttributeDescription position{};
		position.binding = 0;
		position.location = 0;
		position.format = VK_FORMAT_R32G32_SFLOAT;
		position.offset = offsetof(resource::Vertex, pos);

		VkVertexInputAttributeDescription color{};
		color.binding = 0;
		color.location = 1;
		color.format = VK_FORMAT_R32G32B32_SFLOAT;
		color.offset = offsetof(resource::Vertex, color);

		VkVertexInputAttributeDescription vertex_attribute_desc[] = { position, color };

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexAttributeDescriptionCount = 2;
		vertexInputInfo.vertexBindingDescriptionCount = 1;
		vertexInputInfo.pVertexAttributeDescriptions = vertex_attribute_desc;
		vertexInputInfo.pVertexBindingDescriptions = &binding_description;

		VkPipelineInputAssemblyStateCreateInfo input_assembly_state{};
		input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		input_assembly_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		input_assembly_state.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewport_state{};
		viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewport_state.viewportCount = 1;
		viewport_state.pViewports = nullptr;
		viewport_state.scissorCount = 1;
		viewport_state.pScissors = nullptr;

		VkPipelineRasterizationStateCreateInfo rasterization_state{};
		rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterization_state.depthClampEnable = VK_FALSE;
		rasterization_state.rasterizerDiscardEnable = VK_FALSE;
		rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;
		rasterization_state.lineWidth = 1.0f;
		rasterization_state.cullMode = VK_CULL_MODE_NONE;
		rasterization_state.frontFace = VK_FRONT_FACE_CLOCKWISE;
		rasterization_state.depthBiasEnable = VK_FALSE;
		rasterization_state.depthBiasConstantFactor = 0.0f;  // Optional
		rasterization_state.depthBiasClamp = 0.0f;           // Optional
		rasterization_state.depthBiasSlopeFactor = 0.0f;     // Optional


		VkPipelineMultisampleStateCreateInfo multisample_state{};
		multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisample_state.sampleShadingEnable = VK_FALSE;
		multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		multisample_state.minSampleShading = 1.0f;           // Optional
		multisample_state.pSampleMask = nullptr;             // Optional
		multisample_state.alphaToCoverageEnable = VK_FALSE;  // Optional
		multisample_state.alphaToOneEnable = VK_FALSE;       // Optional

		VkPipelineColorBlendAttachmentState color_blend_attachment{};
		color_blend_attachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		color_blend_attachment.blendEnable = VK_FALSE;
		color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;              // Optional
		color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;   // Optional
		color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;  // Optional
		color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;              // Optional

		VkPipelineColorBlendStateCreateInfo color_blend_state{};
		color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		color_blend_state.logicOpEnable = VK_FALSE;
		color_blend_state.logicOp = VK_LOGIC_OP_COPY;  // Optional
		color_blend_state.attachmentCount = 1;
		color_blend_state.pAttachments = &color_blend_attachment;
		color_blend_state.blendConstants[0] = 0.0f;  // Optional
		color_blend_state.blendConstants[1] = 0.0f;  // Optional
		color_blend_state.blendConstants[2] = 0.0f;  // Optional
		color_blend_state.blendConstants[3] = 0.0f;  // Optional

		VkPipelineDepthStencilStateCreateInfo depth_stencil_state{};
		depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depth_stencil_state.depthTestEnable = VK_TRUE;
		depth_stencil_state.depthWriteEnable = VK_TRUE;
		depth_stencil_state.depthCompareOp = VK_COMPARE_OP_LESS;
		depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
		depth_stencil_state.minDepthBounds = 0.0f;  // Optional
		depth_stencil_state.maxDepthBounds = 1.0f;  // Optional
		depth_stencil_state.stencilTestEnable = VK_FALSE;
		depth_stencil_state.front = {};  // Optional
		depth_stencil_state.back = {};   // Optional

		VkPipelineDynamicStateCreateInfo dynamic_state{};
		std::vector<VkDynamicState> dynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamic_state.pDynamicStates = dynamicStateEnables.data();
		dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamicStateEnables.size());
		dynamic_state.flags = 0;

		// TEMP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pushConstantRangeCount = 0;

		if (vkCreatePipelineLayout(device_->GetDeviceHandle(), &pipelineLayoutInfo, nullptr, &pipeline_layout_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout!");
		}

		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2;
		pipelineInfo.pStages = shaderStages;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &input_assembly_state;
		pipelineInfo.pViewportState = &viewport_state;
		pipelineInfo.pRasterizationState = &rasterization_state;
		pipelineInfo.pMultisampleState = &multisample_state;
		pipelineInfo.pColorBlendState = &color_blend_state;
		pipelineInfo.pDepthStencilState = &depth_stencil_state;
		pipelineInfo.pDynamicState = &dynamic_state;

		pipelineInfo.layout = pipeline_layout_;
		pipelineInfo.renderPass = (VkRenderPass)desc.render_pass->GetHandle();
		pipelineInfo.subpass = desc.subpass;

		pipelineInfo.basePipelineIndex = -1;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(
			device_->GetDeviceHandle(),
			VK_NULL_HANDLE,
			1,
			&pipelineInfo,
			nullptr,
			&pipeline_) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create graphics pipeline");
		}
	}

	void VulkanPipeline::CreateShaderModule(const std::vector<char>& code, VkShaderModule* shader_module)
	{
		VkShaderModuleCreateInfo shader_module_create_info{};
		shader_module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shader_module_create_info.codeSize = code.size();
		shader_module_create_info.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkResult result;
		result = vkCreateShaderModule(device_->GetDeviceHandle(), &shader_module_create_info, nullptr, shader_module);
		if (result != VK_SUCCESS)
		{
			MLE_CORE_ERROR("Failed to create ShaderModule");
		}
	}
}