#include "mlepch.h"
#include "VulkanQueue.h"
#include "VulkanDevice.h"

namespace engine {
	VulkanQueue::VulkanQueue(VulkanDevice* in_device, uint32_t index)
		:queue_(VK_NULL_HANDLE), family_index_(index), device_(in_device)
	{
		vkGetDeviceQueue(device_->GetDeviceHandle(), family_index_, 0, &queue_);
	}

	
}