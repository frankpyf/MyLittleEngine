#pragma once
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"
#include "Runtime/Function/RHI/RHIResource.h"
namespace rhi {
	class VulkanRHI;

	class VulkanTexture2D : public RHITexture2D
	{
		friend class VulkanTransferEncoder;
	public:
		VulkanTexture2D(VulkanRHI& in_rhi, const RHITexture2D::Descriptor& desc);
		VulkanTexture2D(VulkanRHI& in_rhi, std::string_view path, uint32_t miplevels);
		virtual ~VulkanTexture2D();

		virtual void SetData(const void* data) override;
		virtual void* GetTextureID() override 
		{ 
			if (descriptor_set_ == nullptr)
			{
				// Create the Descriptor Set:
				descriptor_set_ = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(sampler_, image_view_, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
			return (void*)descriptor_set_; 
		};
		virtual void Resize(uint32_t width, uint32_t height) override;
		virtual void* GetView() override { return (void*)image_view_; };
	private:
		void AllocateMemory(VkImageUsageFlags usage);
		void Release();
	private:
		VulkanRHI& rhi_;

		VkImage			image_ = nullptr;
		VkImageView		image_view_ = nullptr;
		VkSampler		sampler_ = nullptr;
		VkFormat		vk_format_;

		VkImageUsageFlags vk_usage_ = 0;

		VkDescriptorSet descriptor_set_ = nullptr;

		// According to the docs, VmaAllocation represents its underlying memory
		// Since we use vma, it seems that VkDeviceMemory is no longer needed
		VmaAllocation image_allocation_;
	};

	// ---------------------------------------------------

	struct VulkanBuffer : public RHIBuffer
	{
		VkBuffer buffer = VK_NULL_HANDLE;
		VmaAllocation buffer_allocation = VK_NULL_HANDLE;
		VmaAllocationInfo alloc_info;

		VkDescriptorBufferInfo buffer_info;

		virtual void SetData(const void* data, uint64_t size) override;
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

