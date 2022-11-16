#pragma once
#include <vulkan/vulkan.h>
#include "VulkanSwapChain.h"
namespace engine{
	class Window;
}

namespace rhi {
	class VulkanDevice;
	class VulkanCommandBuffer;
	class VulkanRHI;

	class VulkanViewport
	{
	public:
		VulkanViewport(VulkanRHI* in_rhi, VulkanDevice* in_device, engine::Window* in_window);
		~VulkanViewport();

		void Destroy();

		inline VulkanSwapChain* GetSwapChain() { return swap_chain_; }
		uint32_t GetViewportWidth(); 
		uint32_t GetViewportHeight();

		void CreateWindowSurface();
		void CreateSwapChain(VulkanSwapChainRecreateInfo* recreate_info);
		void CleanupSwapChain();
		void RecreateSwapChain();

		void Present(Semaphore** semaphores, uint32_t semaphore_count);

		void AcquireNextImage(VkSemaphore& image_available_semaphore);
		uint32_t GetAccquiredIndex() { return acquired_image_index_; };
	protected:
		VulkanRHI* rhi_;

		VulkanSwapChain* swap_chain_;
		VulkanDevice* device_;

		engine::Window* window_handle_;
		VkSurfaceKHR surface_;
		uint32_t acquired_image_index_;
	};
}
