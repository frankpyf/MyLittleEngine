#include "mlepch.h"
#include "VulkanResource.h"
#include "VulkanRHI.h"
#include "VulkanUtils.h"

#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"

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
    //void VulkanTexture2D::SetData(const void* data)
    //{
    //    VkDeviceSize image_size = width_ * height_ * Utils::BytesPerPixel(vk_format_);

    //    // Create a buffer in host visible memory 
    //    // so that we can use vkMapMemory and copy the pixels to it
    //    VkBuffer staging_buffer;
    //    VkDeviceMemory staging_buffer_memory;
    //    // Create the Upload Buffer
    //    VulkanDevice* device = rhi_.GetDevice();
    //    VulkanUtils::CreateBuffer(device, image_size,
    //                              VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    //                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    //                              staging_buffer,
    //                              staging_buffer_memory);
    //    void* map;
    //    vkMapMemory(device->GetDeviceHandle(), staging_buffer_memory, 0, image_size, 0, &map);
    //    memcpy(map, data, static_cast<size_t>(image_size));
    //    vkUnmapMemory(device->GetDeviceHandle(), staging_buffer_memory);
    //    
    //    VulkanUtils::TransitionImageLayout(device,
    //                                       image_,
    //                                       VK_IMAGE_LAYOUT_UNDEFINED,
    //                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //                                       1,
    //                                       1,
    //                                       VK_IMAGE_ASPECT_COLOR_BIT);
    //    VulkanUtils::CopyBufferToImage(device, staging_buffer, image_, width_, height_, 1);
    //    VulkanUtils::TransitionImageLayout(device, 
    //                                       image_,
    //                                       VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
    //                                       VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
    //                                       1,
    //                                       1,
    //                                       VK_IMAGE_ASPECT_COLOR_BIT);
    //    vkDestroyBuffer(rhi_.GetDevice()->GetDeviceHandle(), staging_buffer, nullptr);
    //    vkFreeMemory(rhi_.GetDevice()->GetDeviceHandle(), staging_buffer_memory, nullptr);
    //}

    void VulkanTexture::RegisterForImGui()
    {
        if(!texture_id)
            texture_id = ImGui_ImplVulkan_AddTexture(sampler, image_view, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }

    // ---------------------------------------------------------------------------------

    void VulkanBuffer::SetData(const void* data, uint64_t size, uint64_t offset)
    {
        if (alloc_info.pMappedData == nullptr)
        {
            MLE_CORE_ERROR("Buffer isn't mapped, can't set data directly. Please use a staging buffer to upload data");
            return;
        }
        char* offset_mapped = (char*)alloc_info.pMappedData;
        offset_mapped += offset;
        memcpy((void*)offset_mapped, data, size);
    }
}