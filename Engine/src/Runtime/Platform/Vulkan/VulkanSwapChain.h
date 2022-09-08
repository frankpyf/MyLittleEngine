/**
* referencing https://github.com/blurrypiano/littleVulkanEngine/blob/main/src/lve_swap_chain.hpp
*/
#pragma once
#include <vulkan/vulkan.h>

namespace engine {
	class VulkanDevice;

	class VulkanSwapChain
	{
		
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

		VulkanSwapChain(VkInstance in_instance, VulkanDevice& in_device, VkExtent2D extent);
		VulkanSwapChain(VkInstance in_instance, VulkanDevice& in_device, VkExtent2D extent, std::shared_ptr<VulkanSwapChain> previous);
		~VulkanSwapChain();
		
		void Destroy();

		VulkanSwapChain(const VulkanSwapChain&) = delete;
		void operator=(const VulkanSwapChain&) = delete;

		VkFramebuffer	GetFrameBuffer(int index)	{ return swap_chain_framebuffers_[index]; }
		VkRenderPass	GetRenderPass()				{ return render_pass_; }
		VkImageView		GetImageView(int index)		{ return swap_chain_image_views_[index]; }
		size_t			GetImageCount()				{ return swap_chain_images_.size(); }
		VkFormat		GetImageFormat()			{ return image_format_; }
		VkExtent2D		GetSwapChainExtent()		{ return swap_chain_extent_; }
		uint32_t		GetWidth()					{ return swap_chain_extent_.width; }
		uint32_t		GetHeight()					{ return swap_chain_extent_.height; }

		float ExtentAspectRatio() 
		{
			return static_cast<float>(swap_chain_extent_.width) / static_cast<float>(swap_chain_extent_.height);
		}
		VkFormat FindDepthFormat();

		VkResult AcquireNextImage(uint32_t* imageIndex);
		VkResult SubmitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);

		bool CompareSwapFormats(const VulkanSwapChain& swap_chain) const 
		{
			return swap_chain.depth_format_ == depth_format_ &&
				swap_chain.image_format_ == image_format_;
		}

	protected:
		void Init();
		
		void CreateWindowSurface();
		void CreateSwapChain();
		void CreateImageViews();
		void CreateDepthResources();
		void CreateRenderPass();
		void CreateFramebuffers();
		void CreateSyncObjects();

		// Helper functions
		VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& available_formats);
		VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& available_present_modes);
		VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

		VkFormat image_format_;
		VkFormat depth_format_;
		VkExtent2D swap_chain_extent_;

		std::vector<VkFramebuffer> swap_chain_framebuffers_;
		VkRenderPass render_pass_;

		std::vector<VkImage> depth_images_;
		std::vector<VkDeviceMemory> depth_image_memorys_;
		std::vector<VkImageView> depth_image_views_;
		std::vector<VkImage> swap_chain_images_;
		std::vector<VkImageView> swap_chain_image_views_;

		VkExtent2D window_extent_;

		VkSwapchainKHR swap_chain_;
		std::shared_ptr<VulkanSwapChain> old_swap_chain_;
		VulkanDevice& device_;

		VkSurfaceKHR surface_;

		VkInstance instance_;

		// to signal that an image has been acquired
		// from the swapchain and is ready for rendering
		std::vector<VkSemaphore> image_available_semaphores_;
		// to signal that rendering has finished and presentation can happen,
		std::vector<VkSemaphore> render_finished_semaphores_;
		// a fence to make sure only 2 frames are rendering at a time.
		std::vector<VkFence> in_flight_fences_;
		std::vector<VkFence> images_in_flight_;
		size_t currentFrame = 0;

	};
}
