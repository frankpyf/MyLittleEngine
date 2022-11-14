#include "mlepch.h"
#include "VulkanSwapChain.h"
#include "VulkanQueue.h"
#include "VulkanUtils.h"
#include "VulkanRenderPass.h"
#include "VulkanRHI.h"
#include "Runtime/Core/Base/Application.h"

namespace rhi {

	VulkanSwapChain::VulkanSwapChain(VkSurfaceKHR in_surface, VulkanDevice& in_device, VulkanSwapChainRecreateInfo* recreate_info)
		:surface_(in_surface),
		device_(in_device),
		swap_chain_(VK_NULL_HANDLE)
	{
		Init(recreate_info);
	}

	VulkanSwapChain::~VulkanSwapChain() 
	{
		swap_chain_ = VK_NULL_HANDLE;
	}

	void VulkanSwapChain::Destroy()
	{
		for (auto it = swapchain_image_views_.begin();it!= swapchain_image_views_.end();it++)
		{
			vkDestroyImageView(device_.GetDeviceHandle(), *it, nullptr);
		}

		if (swap_chain_ != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(device_.GetDeviceHandle(), swap_chain_, nullptr);
		}
	}
	
	VkResult VulkanSwapChain::AcquireNextImage(uint32_t* imageIndex, VkSemaphore &image_available_semaphore)
	{
		VkResult result = vkAcquireNextImageKHR(
			device_.GetDeviceHandle(),
			swap_chain_,
			std::numeric_limits<uint64_t>::max(),
			image_available_semaphore, // semaphore is signalled once the image index is acquired
			VK_NULL_HANDLE,
			imageIndex);

		return result;
	}

	VkResult VulkanSwapChain::Present(Semaphore** semaphores, uint32_t semaphore_count, uint32_t* imageIndex)
	{
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		VkSemaphore* wait_semaphores_vk = new VkSemaphore[semaphore_count];
		for (uint32_t i = 0; i < semaphore_count; ++i)
		{
			VulkanSemaphore* semaphore_vk = (VulkanSemaphore*)semaphores[i];
			wait_semaphores_vk[i] = semaphore_vk->semaphore;
		}
		presentInfo.waitSemaphoreCount = semaphore_count;
		presentInfo.pWaitSemaphores = wait_semaphores_vk;

		VkSwapchainKHR swapChains[] = { swap_chain_ };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = imageIndex;

		auto result = vkQueuePresentKHR(device_.GetPresentQueue()->GetQueueHandle(), &presentInfo);

		delete [] wait_semaphores_vk;
		return result;
	}

	void VulkanSwapChain::Init(VulkanSwapChainRecreateInfo* recreate_info)
	{	
		CreateSwapChain(recreate_info);
		CreateImageViews();
	}

	void VulkanSwapChain::CreateSwapChain(VulkanSwapChainRecreateInfo* recreate_info)
	{
		uint32_t format_count;
		vkGetPhysicalDeviceSurfaceFormatsKHR(device_.GetPhysicalHandle(), surface_, &format_count, nullptr);
		std::vector<VkSurfaceFormatKHR> formats(format_count);
		if (format_count != 0)
		{
			vkGetPhysicalDeviceSurfaceFormatsKHR(device_.GetPhysicalHandle(), surface_, &format_count, formats.data());
		}

		VkSurfaceCapabilitiesKHR capabilities;
		vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_.GetPhysicalHandle(), surface_, &capabilities);

		uint32_t present_mode_count;
		vkGetPhysicalDeviceSurfacePresentModesKHR(device_.GetPhysicalHandle(), surface_, &present_mode_count, nullptr);
		std::vector<VkPresentModeKHR> present_modes(present_mode_count);
		if (present_mode_count != 0)
		{
			vkGetPhysicalDeviceSurfacePresentModesKHR(
				device_.GetPhysicalHandle(),
				surface_,
				&present_mode_count,
				present_modes.data());
		}

		VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(formats);
		VkPresentModeKHR presentMode = ChooseSwapPresentMode(present_modes);
		VkExtent2D extent = ChooseSwapExtent(capabilities, recreate_info->width, recreate_info->height);

		uint32_t imageCount = capabilities.minImageCount + 1;
		MLE_CORE_INFO("This swapchain supports at least {0} image(s)", imageCount);
		if (capabilities.maxImageCount > 0 &&
			imageCount > capabilities.maxImageCount)
		{
			imageCount = capabilities.maxImageCount;
		}

		VkSwapchainCreateInfoKHR createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		createInfo.surface = surface_;

		createInfo.minImageCount = imageCount;
		createInfo.imageFormat = surfaceFormat.format;
		createInfo.imageColorSpace = surfaceFormat.colorSpace;
		createInfo.imageExtent = extent;
		createInfo.imageArrayLayers = 1;
		createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

		device_.SetupPresentQueue(surface_);

		uint32_t queueFamilyIndices[] = { device_.GetGfxQueue()->GetFamilyIndex(), 
										  device_.GetPresentQueue()->GetFamilyIndex()};

		if (device_.GetGfxQueue()->GetFamilyIndex() != 
			device_.GetPresentQueue()->GetFamilyIndex())
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
			createInfo.queueFamilyIndexCount = 2;
			createInfo.pQueueFamilyIndices = queueFamilyIndices;
		}
		else 
		{
			createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
			createInfo.queueFamilyIndexCount = 0;      // Optional
			createInfo.pQueueFamilyIndices = nullptr;  // Optional
		}

		createInfo.preTransform = capabilities.currentTransform;
		createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

		createInfo.presentMode = presentMode;
		createInfo.clipped = VK_TRUE;

		createInfo.oldSwapchain = recreate_info->swap_chain == nullptr ? VK_NULL_HANDLE : recreate_info->swap_chain;

		if (vkCreateSwapchainKHR(device_.GetDeviceHandle(), &createInfo, nullptr, &swap_chain_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}

		// we only specified a minimum number of images in the swap chain, so the implementation is
		// allowed to create a swap chain with more. That's why we'll first query the final number of
		// images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
		// retrieve the handles.
		vkGetSwapchainImagesKHR(device_.GetDeviceHandle(), swap_chain_, &imageCount, nullptr);
		swapchain_images_.resize(imageCount);
		vkGetSwapchainImagesKHR(device_.GetDeviceHandle(), swap_chain_, &imageCount, swapchain_images_.data());

		swapchain_image_format_ = surfaceFormat.format;
		swap_chain_extent_ = extent;
		
		MLE_CORE_INFO("Swapchain created with {0} image(s)", imageCount);
	}

	void VulkanSwapChain::CreateImageViews()
	{
		swapchain_image_views_.resize(GetSwapchainImageCount());
		for (size_t i = 0; i < swapchain_image_views_.size(); ++i)
		{
			VkImageViewCreateInfo view_create_info{};
			view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view_create_info.image = swapchain_images_[i];
			view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view_create_info.format = swapchain_image_format_;
			view_create_info.components.r = VK_COMPONENT_SWIZZLE_R;
			view_create_info.components.g = VK_COMPONENT_SWIZZLE_G;
			view_create_info.components.b = VK_COMPONENT_SWIZZLE_B;
			view_create_info.components.a = VK_COMPONENT_SWIZZLE_A;
			view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			view_create_info.subresourceRange.baseMipLevel = 0;
			view_create_info.subresourceRange.levelCount = 1;
			view_create_info.subresourceRange.baseArrayLayer = 0;
			view_create_info.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device_.GetDeviceHandle(), &view_create_info, nullptr, &swapchain_image_views_[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to create texture image view");
		}
	}

	VkSurfaceFormatKHR VulkanSwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats)
	{
		for (const auto& available_format : available_formats) 
		{
			if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB &&
				available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) 
			{
				return available_format;
			}
		}

		return available_formats[0];
	}

	VkPresentModeKHR VulkanSwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes)
	{
		for (const auto& available_present_mode : available_present_modes)
		{
			if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR)
			{
				MLE_CORE_INFO("Present mode: Mailbox");
				return available_present_mode;
			}
		}

		// for (const auto &availablePresentMode : availablePresentModes) {
		//   if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
		//     std::cout << "Present mode: Immediate" << std::endl;
		//     return availablePresentMode;
		//   }
		// }

		MLE_CORE_INFO("Present mode: FIFO");
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanSwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
		{
			return capabilities.currentExtent;
		}
		else 
		{
			VkExtent2D actual_extent{ width, height };
			actual_extent.width = std::max(
				capabilities.minImageExtent.width,
				std::min(capabilities.maxImageExtent.width, actual_extent.width));
			actual_extent.height = std::max(
				capabilities.minImageExtent.height,
				std::min(capabilities.maxImageExtent.height, actual_extent.height));

			return actual_extent;
		}
	}
}