#pragma once
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

#include "Runtime/Function/RHI/RHIResource.h"
namespace rhi {
	class VulkanRHI;
	class VulkanTexture2D : public RHITexture2D
	{
	public:
		VulkanTexture2D(VulkanRHI& in_rhi, uint32_t width, uint32_t height, PixelFormat in_format, uint32_t miplevels = 0);
		VulkanTexture2D(VulkanRHI& in_rhi, std::string_view path, uint32_t miplevels);
		virtual ~VulkanTexture2D();

		virtual void SetData(const void* data) override;
		virtual void* GetTextureID() const override { return (void*)descriptor_set_; };
		virtual void Resize(uint32_t width, uint32_t height) override;
		inline VkImageView GetImageView() const { return image_view_; };
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
		
		uint32_t miplevels_;

		VulkanRHI& rhi_;
	};
}

