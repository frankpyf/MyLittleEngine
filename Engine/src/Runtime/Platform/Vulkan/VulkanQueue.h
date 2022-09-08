#pragma once
#include <vulkan/vulkan.h>

namespace engine {
	class VulkanDevice;

	class VulkanQueue
	{
	public:
		VulkanQueue(VulkanDevice* in_device, uint32_t index);
		~VulkanQueue() {};
		inline uint32_t GetFamilyIndex() const { return family_index_; };
		inline VkQueue  GetQueueHandle() const { return queue_; };

	private:
		VkQueue queue_;
		uint32_t family_index_;
		VulkanDevice* device_;
	};
}
