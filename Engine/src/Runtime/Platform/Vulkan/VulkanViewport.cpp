#include "mlepch.h"
#include "VulkanViewport.h"
#include "VulkanDevice.h"
#include "VulkanRHI.h"
#include "Runtime/Core/Window.h"
#include "Runtime/Core/Base/Log.h"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>

namespace rhi {
	VulkanViewport::VulkanViewport(VulkanRHI* in_rhi, VulkanDevice* in_device, engine::Window* in_window)
		:rhi_(in_rhi),
		swap_chain_(nullptr),
		device_(in_device),
		window_handle_(in_window),
		surface_(VK_NULL_HANDLE),
		acquired_image_index_(-1)
	{
		CreateWindowSurface();
		VulkanSwapChainRecreateInfo recreate_info = { VK_NULL_HANDLE, surface_, window_handle_->GetWidth(),window_handle_->GetHeight() };
		CreateSwapChain(&recreate_info);
	}
	
	VulkanViewport::~VulkanViewport()
	{
		swap_chain_ = nullptr;
		surface_ = VK_NULL_HANDLE;
	}

	void VulkanViewport::Destroy()
	{
		swap_chain_->Destroy();
		vkDestroySurfaceKHR(rhi_->GetVkInstance(), surface_, nullptr);
	}

	void VulkanViewport::CreateWindowSurface()
	{
		GLFWwindow* window = static_cast<GLFWwindow*>(window_handle_->GetNativeWindow());
		if (glfwCreateWindowSurface(rhi_->GetVkInstance(), window, nullptr, &surface_) != VK_SUCCESS)
		{
			MLE_CORE_ERROR("failed to create window surface!");
			throw std::runtime_error("failed to create window surface!");
		}
	}

	void VulkanViewport::RecreateSwapChain()
	{
		int width = 0, height = 0;
		GLFWwindow* window = static_cast<GLFWwindow*>(window_handle_->GetNativeWindow());
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0) {
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}
		VulkanSwapChainRecreateInfo recreate_info = {swap_chain_->GetSwapchainHandle(), surface_, width, height};
		CreateSwapChain(&recreate_info);
	}

	void VulkanViewport::CreateSwapChain(VulkanSwapChainRecreateInfo* recreate_info)
	{

		if (recreate_info->swap_chain == nullptr)
			swap_chain_ = new VulkanSwapChain(surface_, *device_, recreate_info);
		else
		{
			VulkanSwapChain* old_swap_chain = swap_chain_;
			swap_chain_ = new VulkanSwapChain(surface_, *device_, recreate_info);

			if (!swap_chain_->CompareSwapFormats(*old_swap_chain))
			{
				MLE_CORE_ERROR("Swap chain image(or depth) format has changed!");
				throw std::runtime_error("Swap chain image(or depth) format has changed!");
			}

			old_swap_chain->Destroy();
			delete old_swap_chain;
			old_swap_chain = nullptr;
		}
	}

	void VulkanViewport::CleanupSwapChain()
	{
		rhi_->RHIBlockUntilGPUIdle();
		swap_chain_->Destroy();
	}

	void VulkanViewport::Present(VkSemaphore render_finished_semaphore)
	{
		auto result = swap_chain_->Present(render_finished_semaphore, &acquired_image_index_);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) 
		{
			RecreateSwapChain();
		}
		else if (result != VK_SUCCESS)
		{
			MLE_CORE_ERROR("failed to present swap chain image!");
			throw std::runtime_error("failed to present swap chain image!");
		}
	}

	void VulkanViewport::AcquireNextImage(VkSemaphore& image_available_semaphore)
	{
		auto result = swap_chain_->AcquireNextImage(&acquired_image_index_, image_available_semaphore);
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			RecreateSwapChain();
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
		{
			MLE_CORE_ERROR("failed to acquire swap chain image!");
			throw std::runtime_error("failed to acquire swap chain image!");
		}
	}
}