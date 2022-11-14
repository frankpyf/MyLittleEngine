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
		VulkanTexture2D(VulkanRHI& in_rhi, uint32_t width, uint32_t height, PixelFormat in_format, uint32_t miplevels);
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
		void AllocateMemory(uint64_t size, VkImageUsageFlags usage);
		void Release();
	private:
		VkImage        image_ = nullptr;
		VkImageView    image_view_ = nullptr;
		PixelFormat format_ = PixelFormat::Unknown;
		VkSampler sampler_ = nullptr;

		VkDescriptorSet descriptor_set_ = nullptr;

		// According to the docs, VmaAllocation represents its underlying memory
		// Since we use vma, it seems that VkDeviceMemory is no longer needed
		VmaAllocation image_allocation_;
		
		uint32_t miplevels_ = 1;

		VulkanRHI& rhi_;
	};

	class VulkanBufferBase : public RHIBuffer
	{
		friend class VulkanTransferEncoder;
	public:
		VulkanBufferBase(VulkanRHI& in_rhi)
			:rhi_(in_rhi) {};
		virtual ~VulkanBufferBase();

		void SetData(const void* data, uint64_t size);
	protected:
		VkBuffer buffer_ = VK_NULL_HANDLE;
		VmaAllocation buffer_allocation_;
		
		VulkanRHI& rhi_;
	};

	class VulkanVertexBuffer : public VulkanBufferBase
	{
	public:
		VulkanVertexBuffer(VulkanRHI& in_rhi);
	};
}

