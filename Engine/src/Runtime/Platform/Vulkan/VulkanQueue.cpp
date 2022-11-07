#include "mlepch.h"
#include "VulkanQueue.h"
#include "VulkanDevice.h"
#include "VulkanFrameResource.h"
#include "Runtime/Function/RHI/CommandBuffer.h"

namespace rhi {
	VulkanQueue::VulkanQueue(VulkanDevice* in_device, uint32_t index)
		:queue_(VK_NULL_HANDLE), family_index_(index), device_(in_device)
	{
		vkGetDeviceQueue(device_->GetDeviceHandle(), family_index_, 0, &queue_);
	}

	void VulkanQueue::Submit(CommandBuffer* cmd_buffer)
	{
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		
		VkSemaphore waitSemaphores[] = { (VkSemaphore)cmd_buffer->GetImageAvailableSemaphore()};
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		VkCommandBuffer command_buffers[] = { (VkCommandBuffer)cmd_buffer->GetNativeGfxHandle()};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = command_buffers;

		VkSemaphore signalSemaphores[] = { (VkSemaphore)cmd_buffer->GetRenderFinishedSemaphore()};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(queue_, 1, &submitInfo, (VkFence)cmd_buffer->GetInFlightFence()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}
	}
}