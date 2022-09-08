#include "mlepch.h"
#include "VulkanSwapChain.h"
#include "VulkanDevice.h"
#include "VulkanQueue.h"
#include "Runtime/Core/Base/Log.h"
#include "Runtime/Core/Base/Application.h"
#include <GLFW/glfw3.h>

namespace engine {

	VulkanSwapChain::VulkanSwapChain(VkInstance in_instance, VulkanDevice& in_device, VkExtent2D extent)
		:
		swap_chain_(VK_NULL_HANDLE),
		instance_(in_instance),
		device_(in_device),
		window_extent_(extent)
	{
		CreateWindowSurface();

		Init();
		device_.SetupPresentQueue(surface_);
	}

	VulkanSwapChain::VulkanSwapChain(VkInstance in_instance, VulkanDevice& in_device, VkExtent2D extent, std::shared_ptr<VulkanSwapChain> previous)
		:instance_(in_instance), device_(in_device), window_extent_(extent)
	{
		CreateWindowSurface();

		Init();
		device_.SetupPresentQueue(surface_);
	}

	VulkanSwapChain::~VulkanSwapChain() 
	{
		Destroy();
	}

	void VulkanSwapChain::Destroy()
	{
		vkDeviceWaitIdle(device_.GetDeviceHandle());
		for (auto image_view : swap_chain_image_views_)
		{
			vkDestroyImageView(device_.GetDeviceHandle(), image_view, nullptr);
		}
		swap_chain_image_views_.clear();

		if (swap_chain_ != VK_NULL_HANDLE)
		{
			vkDestroySwapchainKHR(device_.GetDeviceHandle(), swap_chain_, nullptr);
			swap_chain_ = VK_NULL_HANDLE;
		}

		for (int i = 0; i < depth_images_.size(); i++) {
			vkDestroyImageView(device_.GetDeviceHandle(), depth_image_views_[i], nullptr);
			vkDestroyImage(device_.GetDeviceHandle(), depth_images_[i], nullptr);
			vkFreeMemory(device_.GetDeviceHandle(), depth_image_memorys_[i], nullptr);
		}

		for (auto framebuffer : swap_chain_framebuffers_) {
			vkDestroyFramebuffer(device_.GetDeviceHandle(), framebuffer, nullptr);
		}

		vkDestroyRenderPass(device_.GetDeviceHandle(), render_pass_, nullptr);

		// cleanup synchronization objects
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(device_.GetDeviceHandle(), render_finished_semaphores_[i], nullptr);
			vkDestroySemaphore(device_.GetDeviceHandle(), image_available_semaphores_[i], nullptr);
			vkDestroyFence(device_.GetDeviceHandle(), in_flight_fences_[i], nullptr);
		}

		vkDestroySurfaceKHR(instance_, surface_, nullptr);
		surface_ = VK_NULL_HANDLE;
	}

	VkResult VulkanSwapChain::AcquireNextImage(uint32_t* imageIndex) 
	{
		vkWaitForFences(
			device_.GetDeviceHandle(),
			1,
			&in_flight_fences_[currentFrame],
			VK_TRUE,
			std::numeric_limits<uint64_t>::max());

		VkResult result = vkAcquireNextImageKHR(
			device_.GetDeviceHandle(),
			swap_chain_,
			std::numeric_limits<uint64_t>::max(),
			image_available_semaphores_[currentFrame],  // must be a not signaled semaphore
			VK_NULL_HANDLE,
			imageIndex);

		return result;
	}

	VkResult VulkanSwapChain::SubmitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex)
	{
		if (images_in_flight_[*imageIndex] != VK_NULL_HANDLE)
		{
			vkWaitForFences(device_.GetDeviceHandle(), 1, &images_in_flight_[*imageIndex], VK_TRUE, UINT64_MAX);
		}
		images_in_flight_[*imageIndex] = in_flight_fences_[currentFrame];

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

		VkSemaphore waitSemaphores[] = { image_available_semaphores_[currentFrame] };
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = buffers;

		VkSemaphore signalSemaphores[] = { render_finished_semaphores_[currentFrame] };
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		vkResetFences(device_.GetDeviceHandle(), 1, &in_flight_fences_[currentFrame]);
		if (vkQueueSubmit(device_.GetGfxQueue()->GetQueueHandle(), 1, &submitInfo, in_flight_fences_[currentFrame]) !=
			VK_SUCCESS) 
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}

		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = signalSemaphores;

		VkSwapchainKHR swapChains[] = { swap_chain_ };
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = swapChains;

		presentInfo.pImageIndices = imageIndex;

		auto result = vkQueuePresentKHR(device_.GetPresentQueue()->GetQueueHandle(), &presentInfo);

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

		return result;
	}

	void VulkanSwapChain::CreateWindowSurface()
	{
		Application& app = Application::GetApp();
		GLFWwindow* glfw_window_handle = static_cast<GLFWwindow*>(app.GetWindow().GetNativeWindow());
		if (glfwCreateWindowSurface(instance_, glfw_window_handle, nullptr, &surface_) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface!");
		}

	}

	void VulkanSwapChain::Init() {
		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		CreateDepthResources();
		CreateFramebuffers();
		CreateSyncObjects();
	}

	void VulkanSwapChain::CreateSwapChain()
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
		VkExtent2D extent = ChooseSwapExtent(capabilities);

		uint32_t imageCount = capabilities.minImageCount + 1;
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

		createInfo.oldSwapchain = old_swap_chain_ == nullptr ? VK_NULL_HANDLE : old_swap_chain_->swap_chain_;

		if (vkCreateSwapchainKHR(device_.GetDeviceHandle(), &createInfo, nullptr, &swap_chain_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create swap chain!");
		}

		// we only specified a minimum number of images in the swap chain, so the implementation is
		// allowed to create a swap chain with more. That's why we'll first query the final number of
		// images with vkGetSwapchainImagesKHR, then resize the container and finally call it again to
		// retrieve the handles.
		vkGetSwapchainImagesKHR(device_.GetDeviceHandle(), swap_chain_, &imageCount, nullptr);
		swap_chain_images_.resize(imageCount);
		vkGetSwapchainImagesKHR(device_.GetDeviceHandle(), swap_chain_, &imageCount, swap_chain_images_.data());

		image_format_ = surfaceFormat.format;
		swap_chain_extent_ = extent;
	}

	void VulkanSwapChain::CreateImageViews()
	{
		swap_chain_image_views_.resize(swap_chain_images_.size());
		for (size_t i = 0;i<swap_chain_images_.size();++i)
		{
			VkImageViewCreateInfo view_create_info{};
			view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view_create_info.image = swap_chain_images_[i];
			view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view_create_info.format = image_format_;
			view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			view_create_info.subresourceRange.baseMipLevel = 0;
			view_create_info.subresourceRange.levelCount = 1;
			view_create_info.subresourceRange.baseArrayLayer = 0;
			view_create_info.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device_.GetDeviceHandle(), &view_create_info, nullptr, &swap_chain_image_views_[i]) != VK_SUCCESS)
				throw std::runtime_error("failed to create texture image view");
		}
	}

	void VulkanSwapChain::CreateRenderPass()
	{
		VkAttachmentDescription depth_attachment{};
		depth_attachment.format = FindDepthFormat();
		depth_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		depth_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		depth_attachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		depth_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		depth_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		depth_attachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentReference depth_attachment_ref{};
		depth_attachment_ref.attachment = 1;
		depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

		VkAttachmentDescription color_attachment = {};
		color_attachment.format = GetImageFormat();
		color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
		color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment_ref = {};
		color_attachment_ref.attachment = 0;
		color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment_ref;
		subpass.pDepthStencilAttachment = &depth_attachment_ref;

		VkSubpassDependency dependency = {};
		dependency.dstSubpass = 0;
		dependency.dstAccessMask =
			VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
		dependency.dstStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.srcAccessMask = 0;
		dependency.srcStageMask =
			VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;

		std::array<VkAttachmentDescription, 2> attachments = { color_attachment, depth_attachment };
		VkRenderPassCreateInfo renderPassInfo = {};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		renderPassInfo.pAttachments = attachments.data();
		renderPassInfo.subpassCount = 1;
		renderPassInfo.pSubpasses = &subpass;
		renderPassInfo.dependencyCount = 1;
		renderPassInfo.pDependencies = &dependency;

		if (vkCreateRenderPass(device_.GetDeviceHandle(), &renderPassInfo, nullptr, &render_pass_) != VK_SUCCESS) 
		{
			throw std::runtime_error("failed to create render pass!");
		}
	}

	void VulkanSwapChain::CreateDepthResources()
	{
		VkFormat depth_format = FindDepthFormat();
		depth_format_ = depth_format;
		VkExtent2D swap_chain_extent = GetSwapChainExtent();

		depth_images_.resize(GetImageCount());
		depth_image_memorys_.resize(GetImageCount());
		depth_image_views_.resize(GetImageCount());

		for (int i = 0; i < depth_images_.size(); i++) 
		{
			VkImageCreateInfo image_info{};
			image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			image_info.imageType = VK_IMAGE_TYPE_2D;
			image_info.extent.width = swap_chain_extent.width;
			image_info.extent.height = swap_chain_extent.height;
			image_info.extent.depth = 1;
			image_info.mipLevels = 1;
			image_info.arrayLayers = 1;
			image_info.format = depth_format;
			image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
			image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			image_info.samples = VK_SAMPLE_COUNT_1_BIT;
			image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			image_info.flags = 0;

			device_.CreateImageWithInfo(
				image_info,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				depth_images_[i],
				depth_image_memorys_[i]);

			VkImageViewCreateInfo view_info{};
			view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			view_info.image = depth_images_[i];
			view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
			view_info.format = depth_format;
			view_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
			view_info.subresourceRange.baseMipLevel = 0;
			view_info.subresourceRange.levelCount = 1;
			view_info.subresourceRange.baseArrayLayer = 0;
			view_info.subresourceRange.layerCount = 1;

			if (vkCreateImageView(device_.GetDeviceHandle(), &view_info, nullptr, &depth_image_views_[i]) != VK_SUCCESS) 
			{
				throw std::runtime_error("failed to create texture image view!");
			}
		}
	}

	void VulkanSwapChain::CreateFramebuffers()
	{
		swap_chain_framebuffers_.resize(GetImageCount());
		for (size_t i = 0; i < GetImageCount(); i++)
		{
			std::array<VkImageView, 2> attachments = { swap_chain_image_views_[i], depth_image_views_[i] };

			VkExtent2D swap_chain_extent = GetSwapChainExtent();
			VkFramebufferCreateInfo framebufferInfo = {};
			framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
			framebufferInfo.renderPass = render_pass_;
			framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
			framebufferInfo.pAttachments = attachments.data();
			framebufferInfo.width = swap_chain_extent.width;
			framebufferInfo.height = swap_chain_extent.height;
			framebufferInfo.layers = 1;

			if (vkCreateFramebuffer(
				device_.GetDeviceHandle(),
				&framebufferInfo,
				nullptr,
				&swap_chain_framebuffers_[i]) != VK_SUCCESS) 
			{
				throw std::runtime_error("failed to create framebuffer!");
			}
		}
	}

	void VulkanSwapChain::CreateSyncObjects()
	{
		image_available_semaphores_.resize(MAX_FRAMES_IN_FLIGHT);
		render_finished_semaphores_.resize(MAX_FRAMES_IN_FLIGHT);
		in_flight_fences_.resize(MAX_FRAMES_IN_FLIGHT);
		images_in_flight_.resize(GetImageCount(), VK_NULL_HANDLE);

		VkSemaphoreCreateInfo semaphore_info = {};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_info = {};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			if (vkCreateSemaphore(device_.GetDeviceHandle(), &semaphore_info, nullptr, &image_available_semaphores_[i]) !=
				VK_SUCCESS ||
				vkCreateSemaphore(device_.GetDeviceHandle(), &semaphore_info, nullptr, &render_finished_semaphores_[i]) !=
				VK_SUCCESS ||
				vkCreateFence(device_.GetDeviceHandle(), &fence_info, nullptr, &in_flight_fences_[i]) != VK_SUCCESS)
			{
				throw std::runtime_error("failed to create synchronization objects for a frame!");
			}
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

		MLE_CORE_INFO("Present mode: V-Sync");
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D VulkanSwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
	{
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) 
		{
			return capabilities.currentExtent;
		}
		else 
		{
			VkExtent2D actual_extent = window_extent_;
			actual_extent.width = std::max(
				capabilities.minImageExtent.width,
				std::min(capabilities.maxImageExtent.width, actual_extent.width));
			actual_extent.height = std::max(
				capabilities.minImageExtent.height,
				std::min(capabilities.maxImageExtent.height, actual_extent.height));

			return actual_extent;
		}
	}

	VkFormat VulkanSwapChain::FindDepthFormat() {
		return device_.FindSupportedFormat(
			{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
			VK_IMAGE_TILING_OPTIMAL,
			VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
	}
}