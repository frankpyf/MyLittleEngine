#include "mlepch.h"
#include "VulkanResource.h"
#include "VulkanRHI.h"
#include "VulkanUtils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
namespace Utils {
    static uint32_t BytesPerPixel(VkFormat format)
    {
        switch (format)
        {
        case VK_FORMAT_R8G8B8A8_UNORM:      return 4;
        case VK_FORMAT_R32G32B32A32_SFLOAT: return 16;
        }
        return 0;
    }
}
namespace rhi {
    

    VulkanTexture2D::VulkanTexture2D(VulkanRHI& in_rhi, const RHITexture2D::Descriptor& desc)
        :RHITexture2D(desc.width, desc.height, desc.miplevels, desc.format, desc.usage),
        rhi_(in_rhi),
        vk_format_(format_ == PixelFormat::DEPTH? rhi_.GetDepthFormat() : VulkanUtils::MLEFormatToVkFormat(format_))
    {
        if (EnumHasFlag(usage_,  TextureUsage::TRANSIENT_ATTACHMENT))
        {
            vk_usage_ |= VK_IMAGE_USAGE_TRANSIENT_ATTACHMENT_BIT;
        }
        if (EnumHasFlag(usage_, TextureUsage::COLOR_ATTACHMENT))
        {
            vk_usage_ |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        }
        if (EnumHasFlag(usage_, TextureUsage::DEPTH_ATTACHMENT))
        {
            vk_usage_ |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
        if (EnumHasFlag(usage_, TextureUsage::STENCIL_ATTACHMENT))
        {
            vk_usage_ |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        }
        if (EnumHasFlag(usage_, TextureUsage::UPLOADABLE))
        {
            vk_usage_ |= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;;
        }
        if (EnumHasFlag(usage_, TextureUsage::SAMPLEABLE))
        {
            vk_usage_ |= VK_IMAGE_USAGE_SAMPLED_BIT;
        }

        AllocateMemory(vk_usage_);
    }

	VulkanTexture2D::VulkanTexture2D(VulkanRHI& in_rhi, std::string_view path, uint32_t miplevels)
        :RHITexture2D(path, miplevels), rhi_(in_rhi)
	{
        int width, height, channels;
        void* data = nullptr;

        if (stbi_is_hdr(file_path_.c_str()))
        {
            data = (uint8_t*)stbi_loadf(file_path_.c_str(), &width, &height, &channels, 4);
            format_ = PixelFormat::RGBA32F;
        }
        else
        {
            data = stbi_load(file_path_.c_str(), &width, &height, &channels, 4);
            format_ = PixelFormat::RGBA8;
        }

        width_ = width;
        height_ = height;
        vk_format_ = VulkanUtils::MLEFormatToVkFormat(format_);

        if (!data) {
            throw std::runtime_error("failed to load texture image!");
        }

        vk_usage_ = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        AllocateMemory(vk_usage_);

        SetData(data);

        stbi_image_free(data);
	}

    VulkanTexture2D::~VulkanTexture2D()
    {
        Release();
    }

    void VulkanTexture2D::AllocateMemory(VkImageUsageFlags usage)
    {
        VulkanUtils::VMACreateImage(rhi_.allocator_, width_, height_,
            vk_format_,
            VK_IMAGE_TILING_OPTIMAL,
            usage,
            image_,
            1,
            miplevel_,
            image_allocation_);

        // Create Image View
        VkImageViewCreateInfo image_view_create_info{};
        image_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        image_view_create_info.image = image_;
        image_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
        image_view_create_info.format = vk_format_;
        image_view_create_info.subresourceRange.aspectMask = format_ == PixelFormat::DEPTH ? VK_IMAGE_ASPECT_DEPTH_BIT
                                                                                           : VK_IMAGE_ASPECT_COLOR_BIT;
        image_view_create_info.subresourceRange.baseMipLevel = 0;
        image_view_create_info.subresourceRange.levelCount = miplevel_;
        image_view_create_info.subresourceRange.baseArrayLayer = 0;
        image_view_create_info.subresourceRange.layerCount = 1;

        if (vkCreateImageView(rhi_.GetDevice()->GetDeviceHandle(), &image_view_create_info, nullptr, &image_view_) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create image view!");
        }

        VulkanUtils::CreateLinearSampler((VkDevice)rhi_.GetNativeDevice(),
                                         (VkPhysicalDevice)rhi_.GetNativePhysicalDevice(),
                                          sampler_);
    }

    void VulkanTexture2D::Release()
    {
        rhi_.RHIBlockUntilGPUIdle();
        vkDestroySampler((VkDevice)rhi_.GetNativeDevice(), sampler_, nullptr);
        vkDestroyImageView((VkDevice)rhi_.GetNativeDevice(), image_view_, nullptr);
        vmaDestroyImage(rhi_.allocator_, image_, image_allocation_);
        sampler_ = nullptr;
        image_view_ = nullptr;
        image_ = nullptr;
        image_allocation_ = nullptr;
    }

    void VulkanTexture2D::SetData(const void* data)
    {
        VkDeviceSize image_size = width_ * height_ * Utils::BytesPerPixel(vk_format_);

        // Create a buffer in host visible memory 
        // so that we can use vkMapMemory and copy the pixels to it
        VkBuffer staging_buffer;
        VkDeviceMemory staging_buffer_memory;
        // Create the Upload Buffer
        VulkanDevice* device = rhi_.GetDevice();
        VulkanUtils::CreateBuffer(device, image_size,
                                  VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                  staging_buffer,
                                  staging_buffer_memory);
        void* map;
        vkMapMemory(device->GetDeviceHandle(), staging_buffer_memory, 0, image_size, 0, &map);
        memcpy(map, data, static_cast<size_t>(image_size));
        vkUnmapMemory(device->GetDeviceHandle(), staging_buffer_memory);
        
        VulkanUtils::TransitionImageLayout(device,
                                           image_,
                                           VK_IMAGE_LAYOUT_UNDEFINED,
                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                           1,
                                           1,
                                           VK_IMAGE_ASPECT_COLOR_BIT);
        VulkanUtils::CopyBufferToImage(device, staging_buffer, image_, width_, height_, 1);
        VulkanUtils::TransitionImageLayout(device, 
                                           image_,
                                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                                           VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                                           1,
                                           1,
                                           VK_IMAGE_ASPECT_COLOR_BIT);
        vkDestroyBuffer(rhi_.GetDevice()->GetDeviceHandle(), staging_buffer, nullptr);
        vkFreeMemory(rhi_.GetDevice()->GetDeviceHandle(), staging_buffer_memory, nullptr);
    }

    void VulkanTexture2D::Resize(uint32_t width, uint32_t height)
    {
        if (image_ && width_ == width && height_ == height)
            return;
        width_ = width;
        height_ = height;

        Release();

        AllocateMemory(vk_usage_);

        // Create the Descriptor Set for IMGUI:
        descriptor_set_ = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(sampler_, image_view_, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    // ---------------------------------------------------------------------------------

    void VulkanBuffer::SetData(const void* data, uint64_t size)
    {
        if (alloc_info.pMappedData == nullptr)
        {
            MLE_CORE_ERROR("Buffer isn't mapped, can't set data directly. Please use a staging buffer to upload data");
            return;
        }
        memcpy(alloc_info.pMappedData, data, size);
    }
}