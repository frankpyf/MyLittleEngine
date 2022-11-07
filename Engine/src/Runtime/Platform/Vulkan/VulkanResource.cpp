#include "mlepch.h"
#include "VulkanResource.h"
#include "VulkanRHI.h"
#include "VulkanUtils.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

namespace rhi {
    namespace Utils {
        static uint32_t BytesPerPixel(PixelFormat format)
        {
            switch (format)
            {
            case PixelFormat::RGBA:    return 4;
            case PixelFormat::RGBA32F: return 16;
            }
            return 0;
        }
    }

    VulkanTexture2D::VulkanTexture2D(VulkanRHI& in_rhi, uint32_t width, uint32_t height, PixelFormat in_format, uint32_t miplevels)
        :RHITexture2D(width, height), miplevels_(miplevels), format_(in_format), rhi_(in_rhi)
    {
        VkFormat format = VulkanUtils::PixelFormatToVulkanFormat(format_);
        VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
       
        AllocateMemory(width_ * height_ * Utils::BytesPerPixel(format_), usage);
    }

	VulkanTexture2D::VulkanTexture2D(VulkanRHI& in_rhi, std::string_view path, uint32_t miplevels)
        :RHITexture2D(path), miplevels_(miplevels), rhi_(in_rhi)
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
            format_ = PixelFormat::RGBA;
        }

        width_ = width;
        height_ = height;

        if (!data) {
            throw std::runtime_error("failed to load texture image!");
        }

        VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        AllocateMemory(width_ * height_ * Utils::BytesPerPixel(format_), usage);

        SetData(data);

        stbi_image_free(data);
	}

    VulkanTexture2D::~VulkanTexture2D()
    {
        Release();
    }

    void VulkanTexture2D::AllocateMemory(uint64_t size, VkImageUsageFlags usage)
    {
        //Create Image
        VulkanUtils::VMACreateImage(rhi_.allocator_, width_, height_,
            VulkanUtils::PixelFormatToVulkanFormat(format_),
            VK_IMAGE_TILING_OPTIMAL,
            usage,
            image_,
            1,
            miplevels_,
            image_allocation_);
        image_view_ = VulkanUtils::CreateImageView((VkDevice)rhi_.GetNativeDevice(), image_,
                                                   VulkanUtils::PixelFormatToVulkanFormat(format_),
                                                   VK_IMAGE_ASPECT_COLOR_BIT,
                                                   VK_IMAGE_VIEW_TYPE_2D,
                                                   1,
                                                   miplevels_);
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
        VkDeviceSize image_size = width_ * height_ * Utils::BytesPerPixel(format_);

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

        VkImageUsageFlags usage;
        if (file_path_.empty())
            usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
        else
            usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
        AllocateMemory(width_ * height_ * Utils::BytesPerPixel(format_), usage);

        // Create the Descriptor Set:
        descriptor_set_ = (VkDescriptorSet)ImGui_ImplVulkan_AddTexture(sampler_, image_view_, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
}