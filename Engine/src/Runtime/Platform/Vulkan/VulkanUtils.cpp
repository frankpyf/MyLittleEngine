#include "mlepch.h"
#include "VulkanUtils.h"
#include "VulkanRHI.h"
#include "Runtime/Function/RHI/RHIResource.h"

namespace rhi {
    void VulkanUtils::CreateBuffer(VulkanDevice* device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateBuffer(device->GetDeviceHandle(), &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
            throw std::runtime_error("failed to create buffer!");
        }

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(device->GetDeviceHandle(), buffer, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(device->GetPhysicalHandle(), memRequirements.memoryTypeBits, properties);

        if (vkAllocateMemory(device->GetDeviceHandle(), &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
            throw std::runtime_error("failed to allocate buffer memory!");
        }

        vkBindBufferMemory(device->GetDeviceHandle(), buffer, bufferMemory, 0);
    }

    void VulkanUtils::VMACreateBuffer(const VmaAllocator& allocator, VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer& buffer, VmaAllocation& buffer_allocation)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_AUTO;

        vmaCreateBuffer(allocator, &bufferInfo, &allocInfo, &buffer, &buffer_allocation, nullptr);
    }

    void VulkanUtils::VMACreateImage(VmaAllocator& allocator,
                                     uint32_t              image_width,
                                     uint32_t              image_height,
                                     VkFormat              format,
                                     VkImageTiling         image_tiling,
                                     VkImageUsageFlags     image_usage_flags,
                                     VkImage& image,
                                     uint32_t              array_layers,
                                     uint32_t              miplevels,
                                     VmaAllocation& image_allocation)
    {
        // use the vmaAllocator to allocate asset texture image
        VkImageCreateInfo image_create_info{};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.extent.width = image_width;
        image_create_info.extent.height = image_height;
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = miplevels;
        image_create_info.arrayLayers = array_layers;
        image_create_info.format = format;
        image_create_info.tiling = image_tiling;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_create_info.usage = image_usage_flags;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        // TODO: SUPPORT LAZILY ALLOCATED
        VmaAllocationCreateInfo allocInfo = {};
        allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
        
        VkResult result = vmaCreateImage(allocator, &image_create_info, &allocInfo, &image, &image_allocation, NULL);
        if ( result != VK_SUCCESS)
        {
            MLE_CORE_ERROR("failed to create image with VMA!{0}", result);
            throw std::runtime_error("failed to create image with VMA!");
        }
    }

    void VulkanUtils::CreateImage(VulkanDevice* device,
                                  uint32_t              image_width,
                                  uint32_t              image_height,
                                  VkFormat              format,
                                  VkImageTiling         image_tiling,
                                  VkImageUsageFlags     image_usage_flags,
                                  VkMemoryPropertyFlags memory_property_flags,
                                  VkImage& image,
                                  VkDeviceMemory& memory,
                                  uint32_t              array_layers,
                                  uint32_t              miplevels)
    {
        VkImageCreateInfo image_create_info{};
        image_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        image_create_info.imageType = VK_IMAGE_TYPE_2D;
        image_create_info.extent.width = image_width;
        image_create_info.extent.height = image_height;
        image_create_info.extent.depth = 1;
        image_create_info.mipLevels = miplevels;
        image_create_info.arrayLayers = array_layers;
        image_create_info.format = format;
        image_create_info.tiling = image_tiling;
        image_create_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        image_create_info.usage = image_usage_flags;
        image_create_info.samples = VK_SAMPLE_COUNT_1_BIT;
        image_create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

        if (vkCreateImage(device->GetDeviceHandle(), &image_create_info, nullptr, &image) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image!");
        }

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(device->GetDeviceHandle(), image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = FindMemoryType(device->GetPhysicalHandle(), memRequirements.memoryTypeBits, memory_property_flags);

        if (vkAllocateMemory(device->GetDeviceHandle(), &allocInfo, nullptr, &memory) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to allocate image memory!");
        }

        vkBindImageMemory(device->GetDeviceHandle(), image, memory, 0);
    }

    VkImageView VulkanUtils::CreateImageView(const VkDevice& device,
                                             VkImage& image,
                                             VkFormat           format,
                                             VkImageAspectFlags image_aspect_flags,
                                             VkImageViewType    view_type,
                                             uint32_t           layout_count,
                                             uint32_t           miplevels)
    {

        VkImageViewCreateInfo image_view_create_info{};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = image;
        image_view_create_info.viewType = view_type;
        image_view_create_info.format = format;
        image_view_create_info.subresourceRange.aspectMask = image_aspect_flags;
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = miplevels;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = layout_count;

        VkImageView image_view;
        if (vkCreateImageView(device, &image_view_create_info, nullptr, &image_view) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image view!");
        }

        return image_view;
    }

    uint32_t VulkanUtils::FindMemoryType(const VkPhysicalDevice& physical_device, uint32_t type_filter, VkMemoryPropertyFlags properties_flag)
    {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(physical_device, &memProperties);

        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((type_filter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties_flag) == properties_flag) {
                return i;
            }
        }

        throw std::runtime_error("failed to find suitable memory type!");
    }

    VkFormat VulkanUtils::FindSupportedFormat(const VkPhysicalDevice& physical_device, 
                                              const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates)
        {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(physical_device, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }

        throw std::runtime_error("failed to find supported format!");
    }

    VkFormat VulkanUtils::FindDepthFormat(const VkPhysicalDevice& physical_device)
    {
        return FindSupportedFormat(physical_device, 
            { VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
            VK_IMAGE_TILING_OPTIMAL,
            VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT
        );
    }

    VkFormat VulkanUtils::PixelFormatToVulkanFormat(PixelFormat format)
    {
        switch (format)
        {
        case PixelFormat::RGBA: return VK_FORMAT_R8G8B8A8_UNORM;
        case PixelFormat::RGBA32F:return VK_FORMAT_R32G32B32A32_SFLOAT;
        default:return (VkFormat)0;
        }
    }

    void VulkanUtils::TransitionImageLayout(VulkanDevice* in_device,
                                            VkImage            image,
                                            VkImageLayout      old_layout,
                                            VkImageLayout      new_layout,
                                            uint32_t           layer_count,
                                            uint32_t           miplevels,
                                            VkImageAspectFlags aspect_mask_bits)
    {
        VkCommandBuffer commandBuffer = in_device->BeginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = old_layout;
        barrier.newLayout = new_layout;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = image;
        barrier.subresourceRange.aspectMask = aspect_mask_bits;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = miplevels;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = layer_count;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (old_layout == VK_IMAGE_LAYOUT_UNDEFINED && new_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
        {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL &&
            new_layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        // for getGuidAndDepthOfMouseClickOnRenderSceneForUI() get depthimage
        else if (old_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL &&
            new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL &&
            new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
            barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        }
        // for generating mipmapped image
        else if (old_layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && new_layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
        {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else
        {
            throw std::invalid_argument("unsupported layout transition!");
        }

        vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

        in_device->EndSingleTimeCommands(commandBuffer);
    }


    void VulkanUtils::CopyBufferToImage(VulkanDevice* device,
                                        VkBuffer              buffer,
                                        VkImage               image,
                                        uint32_t              width,
                                        uint32_t              height,
                                        uint32_t              layer_count)
    {
        VkCommandBuffer command_buffer = device->BeginSingleTimeCommands();

        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;
        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = layer_count;
        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = { width, height, 1 };

        vkCmdCopyBufferToImage(command_buffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

        device->EndSingleTimeCommands(command_buffer);
    }

    VkSampler VulkanUtils::CreateLinearSampler(const VkDevice& device, const VkPhysicalDevice& physical_device, VkSampler& sampler)
    {
        VkPhysicalDeviceProperties physical_device_properties{};
        vkGetPhysicalDeviceProperties(physical_device, &physical_device_properties);

        VkSamplerCreateInfo samplerInfo{};

        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.anisotropyEnable = VK_FALSE;
        samplerInfo.maxAnisotropy = physical_device_properties.limits.maxSamplerAnisotropy; // close :1.0f
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = 8.0f; // todo: m_irradiance_texture_miplevels
        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;

        if (vkCreateSampler(device, &samplerInfo, nullptr, &sampler) != VK_SUCCESS)
        {
            throw std::runtime_error("vk create sampler");
        }
    }
}