#include "mlepch.h"
#include "VulkanRenderer.h"
#include "VulkanSwapChain.h"

namespace engine {
	void VulkanRenderer::RecreateSwapChain()
	{

	}
	
	void VulkanRenderer::CreateCommandBuffers()
	{
		command_buffers_.resize(VulkanSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = lveDevice.getCommandPool();
		alloc_info.commandBufferCount = static_cast<uint32_t>(command_buffers_.size());
	}
}