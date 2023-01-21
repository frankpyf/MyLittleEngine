#include "mlepch.h"
#include "VulkanQueue.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"
#include "VulkanRHI.h"

namespace rhi {
	VulkanQueue::VulkanQueue(VulkanDevice* in_device, uint32_t index)
		:queue_(VK_NULL_HANDLE), family_index_(index), device_(in_device)
	{
		vkGetDeviceQueue(device_->GetDeviceHandle(), family_index_, 0, &queue_);
	}

	void VulkanQueue::Submit(const QueueSubmitDesc& desc)
	{
		VkSubmitInfo submitInfo {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		
		assert(desc.cmds_count > 0 && "command buffer count must be greater than 0");
		assert(desc.encoders);
		VkCommandBuffer* command_buffers = new VkCommandBuffer[desc.cmds_count];
		for (uint32_t i = 0; i < desc.cmds_count; ++i)
		{
			command_buffers[i] = (VkCommandBuffer)desc.encoders[i]->GetHandle();
		}
		submitInfo.commandBufferCount = desc.cmds_count;
		submitInfo.pCommandBuffers = command_buffers;
		
		VkSemaphore* wait_semaphores = new VkSemaphore[desc.wait_semaphore_count];
		VulkanSemaphore** wait_semaphores_vk = (VulkanSemaphore**)desc.wait_semaphore;
		for (uint32_t i = 0; i < desc.wait_semaphore_count; ++i)
		{
			wait_semaphores[i] = wait_semaphores_vk[i]->semaphore;
		}
		submitInfo.waitSemaphoreCount = desc.wait_semaphore_count;
		submitInfo.pWaitSemaphores = wait_semaphores;
		VkPipelineStageFlags wait_stages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.pWaitDstStageMask = wait_stages;

		VkSemaphore* signal_semaphores = new VkSemaphore[desc.signal_semaphore_count];
		VulkanSemaphore** signal_semaphores_vk = (VulkanSemaphore**)desc.signal_semaphore;
		for (uint32_t i = 0; i < desc.signal_semaphore_count; ++i)
		{
			signal_semaphores[i] = signal_semaphores_vk[i]->semaphore;
		}
		submitInfo.signalSemaphoreCount = desc.signal_semaphore_count;
		submitInfo.pSignalSemaphores = signal_semaphores;

		VulkanFence* fence = (VulkanFence*)desc.signal_fence;
		if (vkQueueSubmit(queue_, 1, &submitInfo, fence ? fence->fence : VK_NULL_HANDLE) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}
		
		delete[] signal_semaphores;
		delete[] wait_semaphores;
		delete[] command_buffers;
	}
}