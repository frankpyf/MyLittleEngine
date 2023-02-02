#pragma once
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

#include "Runtime/Function/RHI/RHIResource.h"
namespace rhi {
	class VulkanRHI;

	struct VulkanTexture : public RHITexture
	{
		VkImage			image = VK_NULL_HANDLE;
		VkImageView		image_view = nullptr;
		VkSampler		sampler = nullptr;
		VkFormat		vk_format;

		VkImageUsageFlags vk_usage = 0;

		VkDescriptorImageInfo texture_info;

		VmaAllocation image_allocation;

		virtual void RegisterForImGui() override;
	};

	// ---------------------------------------------------

	struct VulkanBuffer : public RHIBuffer
	{
		VkBuffer buffer = VK_NULL_HANDLE;
		VmaAllocation buffer_allocation = VK_NULL_HANDLE;
		VmaAllocationInfo alloc_info;

		VkDescriptorBufferInfo buffer_info;

		virtual void SetData(const void* data, uint64_t size, uint64_t offset = 0) override;
	};

	// ---------------------------------------------------

	struct VulkanShaderModule : public ShaderModule
	{
		VkShaderModule shader_module;
	};

	struct VulkanPipelineLayout : public PipelineLayout
	{
		VkPipelineLayout pipeline_layout;
	};

	struct VulkanPipeline : public RHIPipeline
	{
		VkPipeline pipeline;
		VkShaderModule vert_shader;
		VkShaderModule frag_shader;
	};
}

