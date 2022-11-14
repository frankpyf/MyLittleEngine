 #pragma once
#include <vulkan/vulkan.h>

namespace rhi {
	class VulkanDevice;
	class VulgkanCommandBuffer;
	class VulkanRenderPass;
	struct Semaphore;

	struct VulkanSwapChainRecreateInfo
	{
		VkSwapchainKHR swap_chain;
		VkSurfaceKHR surface;
		uint32_t width;
		uint32_t height;
	};

	class VulkanSwapChain
	{
	public:
		VulkanSwapChain(VkSurfaceKHR in_surface, VulkanDevice& in_device, VulkanSwapChainRecreateInfo* recreate_info);
		~VulkanSwapChain();
		
		void Destroy();

		VulkanSwapChain(const VulkanSwapChain&) = delete;
		void operator=(const VulkanSwapChain&) = delete;

		VkSwapchainKHR& GetSwapchainHandle()				{ return swap_chain_; };
		VkImageView		GetSwapchianImageView(int index)	{ return swapchain_image_views_[index]; }
		size_t			GetSwapchainImageCount()			{ return swapchain_images_.size(); }
		VkFormat		GetImageFormat()					{ return swapchain_image_format_; }
		VkExtent2D		GetSwapchainExtent()				{ return swap_chain_extent_; }
		uint32_t		GetSwapchainWidth()					{ return swap_chain_extent_.width; }
		uint32_t		GetSwapchainHeight()				{ return swap_chain_extent_.height; }

		float ExtentAspectRatio() 
		{
			return static_cast<float>(swap_chain_extent_.width) / static_cast<float>(swap_chain_extent_.height);
		}

		VkResult AcquireNextImage(uint32_t* imageIndex, VkSemaphore &image_available_semaphore);
		/**
		* @brief Use the Present Queue to present the rendered image
		* @param signal that the rendering process has finished
		* @param index of the target swapchain image
		*
		* @return the result of vkQueuePresentKHR
		*/
		VkResult Present(Semaphore** semaphores, uint32_t semaphore_count, uint32_t* imageIndex);

		bool CompareSwapFormats(const VulkanSwapChain& swap_chain) const 
		{
			return swap_chain.swapchain_image_format_ == swapchain_image_format_;
		}

	protected:
		void Init(VulkanSwapChainRecreateInfo* recreate_info);
		
		void CreateSwapChain(VulkanSwapChainRecreateInfo* recreate_info);
		void CreateImageViews();

		// Helper functions

		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, uint32_t width, uint32_t height);

		VulkanDevice& device_;
		VkSurfaceKHR& surface_;

		VkSwapchainKHR swap_chain_;
		
		// the size (in pixels) of the swapchain image(s)
		VkExtent2D swap_chain_extent_;

		VkFormat swapchain_image_format_;

		std::vector<VkImage> swapchain_images_;
		std::vector<VkImageView> swapchain_image_views_;
	};
}
