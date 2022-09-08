#include "mlepch.h"
#include "VulkanViewport.h"
#include "VulkanSwapChain.h"
#include "VulkanDevice.h"
#include "VulkanRHI.h"
#include "Runtime/Core/Window.h"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace engine {
	VulkanViewport::VulkanViewport(VulkanRHI* in_rhi, VulkanDevice* in_device, uint32_t in_size_x, uint32_t in_size_y)
		:rhi_(in_rhi),
		device_(in_device),
		size_x_(in_size_x),
		size_y_(in_size_y)
	{
		CreateSwapChain();
	}

	void VulkanViewport::RecreateSwapChain()
	{
		DestroySwapChain();

		CreateSwapChain();
	}

	void VulkanViewport::Resize(uint32_t in_size_x, uint32_t in_size_y)
	{
		if (in_size_x != size_x_ && in_size_y != size_y_)
		{
			size_x_ = in_size_x;
			size_y_ = in_size_y;
			RecreateSwapChain();
		}
	}

	void VulkanViewport::CreateSwapChain()
	{
		if (swap_chain_ != nullptr)
			swap_chain_ = new VulkanSwapChain(rhi_->GetInstance(), *device_, { size_x_, size_y_ });
		else
		{
			VulkanSwapChain* old_swap_chain = swap_chain_;
			swap_chain_ = new VulkanSwapChain(rhi_->GetInstance(), *device_, { size_x_, size_y_ });

			if (!swap_chain_->CompareSwapFormats(*old_swap_chain)) {
				throw std::runtime_error("Swap chain image(or depth) format has changed!");
			}

			old_swap_chain->Destroy();
			delete old_swap_chain;
			old_swap_chain = nullptr;
		}
	}

	void VulkanViewport::DestroySwapChain()
	{
		vkDeviceWaitIdle(device_->GetDeviceHandle());
		swap_chain_->Destroy();
		delete swap_chain_;
		swap_chain_ = nullptr;
	}

	void VulkanViewport::Present(const VkCommandBuffer* buffers, uint32_t* imageIndex)
	{
		auto result = swap_chain_->SubmitCommandBuffers(buffers, imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) 
		{
			Resize(window_handle_->GetWidth(), window_handle_->GetHeight());
		}
		else if (result != VK_SUCCESS)
		{
			throw std::runtime_error("failed to present swap chain image!");
		}
	}

	VkResult VulkanViewport::AcquireNextImage(uint32_t* imageIndex)
	{
		return swap_chain_->AcquireNextImage(imageIndex);
	}
}