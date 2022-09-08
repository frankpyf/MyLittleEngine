#pragma once
#include <vulkan/vulkan.h>
namespace engine {
	class VulkanSwapChain;
	class VulkanDevice;
	class VulkanRHI;
	class Window;

	class VulkanViewport
	{
	public:
		VulkanViewport(VulkanRHI* in_rhi, VulkanDevice* in_device, uint32_t in_size_x, uint32_t in_size_y);

		inline VulkanSwapChain* GetSwapChain() { return swap_chain_; }

		void CreateSwapChain();
		void DestroySwapChain();
		void RecreateSwapChain();
		void Resize(uint32_t in_size_x, uint32_t in_size_y);

		void Present(const VkCommandBuffer* buffers, uint32_t* imageIndex);

		VkResult AcquireNextImage(uint32_t* imageIndex);
		
	protected:
		VulkanRHI* rhi_;
		uint32_t size_x_;
		uint32_t size_y_;

		VulkanSwapChain* swap_chain_;
		VulkanDevice* device_;

		Window* window_handle_;
	};
}
