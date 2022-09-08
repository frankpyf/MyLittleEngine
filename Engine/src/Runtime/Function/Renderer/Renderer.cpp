#include "mlepch.h"
#include "Renderer.h"
#include "Runtime/Platform/Vulkan/VulkanSwapChain.h"
#include "Runtime/Platform/Vulkan/VulkanDevice.h"
#include "Runtime/Platform/Vulkan/VulkanViewport.h"

namespace engine
{
    Renderer::Renderer()
    {}

    void Renderer::Init()
    {
        rhi_ = std::make_unique<VulkanRHI>();
        rhi_->Init();

    }

    void Renderer::Shutdown()
    {
        rhi_->RHIBlockUntilGPUIdle();
        FreeCommandBuffers();
    }

    void Renderer::Tick()
    {

    }

    void Renderer::CreateCommandBuffers() 
    {
        command_buffers_.resize(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = rhi_.get()->GetDevice()->GetCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(command_buffers_.size());

        if (vkAllocateCommandBuffers(
            rhi_.get()->GetDevice()->GetDeviceHandle(), &allocInfo, command_buffers_.data())
            !=VK_SUCCESS) 
        {
            throw std::runtime_error("failed to allocate command buffers!");
        }
    }

    void Renderer::FreeCommandBuffers() 
    {
        vkFreeCommandBuffers(
            rhi_.get()->GetDevice()->GetDeviceHandle(),
            rhi_.get()->GetDevice()->GetCommandPool(),
            static_cast<uint32_t>(command_buffers_.size()),
            command_buffers_.data());
        command_buffers_.clear();
    }

    VkCommandBuffer Renderer::BeginFrame()
    {
        assert(!is_frame_started_ && "Can't call beginFrame while already in progress");

        auto result = rhi_.get()->GetViewport()->AcquireNextImage(&current_image_index_);
        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            rhi_.get()->GetViewport()->RecreateSwapChain();
            return nullptr;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("failed to acquire swap chain image!");
        }

        is_frame_started_ = true;

        auto command_buffer = GetCurrentCommandBuffer();
        VkCommandBufferBeginInfo begin_info{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to begin recording command buffer!");
        }
        return command_buffer;
    }

    void Renderer::EndFrame() 
    {
        assert(is_frame_started_ && "Can't call endFrame while frame is not in progress");
        auto command_buffer = GetCurrentCommandBuffer();
        if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to record command buffer!");
        }

        rhi_.get()->GetViewport()->Present(&command_buffer, &current_image_index_);

        is_frame_started_ = false;
        current_frame_index_ = (current_frame_index_ + 1) % VulkanSwapChain::MAX_FRAMES_IN_FLIGHT;
    }

    void Renderer::Resize(uint32_t width, uint32_t height)
    {
        rhi_->GetViewport()->Resize(width, height);
    }
}