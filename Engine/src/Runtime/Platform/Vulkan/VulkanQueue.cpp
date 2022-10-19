#include "mlepch.h"
#include "VulkanQueue.h"
#include "VulkanDevice.h"
#include "VulkanFrameResource.h"

namespace rhi {
	VulkanQueue::VulkanQueue(VulkanDevice* in_device, uint32_t index)
		:queue_(VK_NULL_HANDLE), family_index_(index), device_(in_device)
	{
		vkGetDeviceQueue(device_->GetDeviceHandle(), family_index_, 0, &queue_);
	}

	void VulkanQueue::Submit(VulkanFrameResource& frame)
	{
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		
		VkSemaphore waitSemaphores[] = { frame.GetImageAcquireSemaphore()};
		VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		submitInfo.waitSemaphoreCount = 1;
		submitInfo.pWaitSemaphores = waitSemaphores;
		submitInfo.pWaitDstStageMask = waitStages;

		VkCommandBuffer command_buffers[] = { frame.GetCommandBuffer()};
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = command_buffers;

		VkSemaphore signalSemaphores[] = { frame.GetRenderFinishedSemaphore()};
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = signalSemaphores;

		if (vkQueueSubmit(queue_, 1, &submitInfo, frame.GetFence()) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to submit draw command buffer!");
		}
	}
}