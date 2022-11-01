#pragma once
#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"
namespace rhi {
	class RHI;
	enum class PixelFormat;
	class VulkanDevice;

	class VulkanUtils
	{
	public:
		static void CreateBuffer(VulkanDevice* device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
		static void VMACreateBuffer(const VmaAllocator& allocator,
									VkDeviceSize size, 
									VkBufferUsageFlags usage, 
									VkMemoryPropertyFlags properties, 
									VkBuffer& buffer,
									VmaAllocation& buffer_allocation);
		static void VMACreateImage(VmaAllocator& allocator,
								   uint32_t              image_width,
								   uint32_t              image_height,
								   VkFormat              format,
								   VkImageTiling         image_tiling,
								   VkImageUsageFlags     image_usage_flags,
								   VkImage& image,
								   uint32_t              array_layers,
								   uint32_t              miplevels,
								   VmaAllocation& image_allocation);
		static void CreateImage(VulkanDevice* device,
								uint32_t              image_width,
								uint32_t              image_height,
								VkFormat              format,
								VkImageTiling         image_tiling,
								VkImageUsageFlags     image_usage_flags,
								VkMemoryPropertyFlags memory_property_flags,
								VkImage& image,
								VkDeviceMemory& memory,
								uint32_t              array_layers,
								uint32_t              miplevels);
		static VkImageView CreateImageView(const VkDevice& device,
										   VkImage& image,
										   VkFormat           format,
										   VkImageAspectFlags image_aspect_flags,
										   VkImageViewType    view_type,
										   uint32_t           layout_count,
										   uint32_t           miplevels);
		// todo: add params, this can only be a temp solution
		static VkSampler CreateLinearSampler(const VkDevice& device,
											 const VkPhysicalDevice& physical_device,
											 VkSampler& sampler);
		static void TransitionImageLayout(VulkanDevice* in_device,
									      VkImage               image,
										  VkImageLayout         old_layout,
										  VkImageLayout         new_layout,
										  uint32_t              layer_count,
										  uint32_t              miplevels,
										  VkImageAspectFlags    aspect_mask_bits);
		static void CopyBufferToImage(VulkanDevice* device,
								      VkBuffer              buffer,
									  VkImage               image,
									  uint32_t              width,
									  uint32_t              height,
									  uint32_t              layer_count);
		static VkFormat FindSupportedFormat(const VkPhysicalDevice& physical_device, 
											const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
		static VkFormat FindDepthFormat(const VkPhysicalDevice& physical_device);
		static uint32_t FindMemoryType(const VkPhysicalDevice& physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties_flag);
		static VkFormat PixelFormatToVulkanFormat(PixelFormat format);
	};
}


