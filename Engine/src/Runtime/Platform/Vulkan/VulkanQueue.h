#pragma once
#include <vulkan/vulkan.h>

namespace rhi {
	class VulkanDevice;
	class VulkanFrameResource;
	class CommandBuffer;
	struct QueueSubmitDesc;

	class VulkanQueue
	{
	public:
		VulkanQueue(VulkanDevice* in_device, uint32_t index);
		~VulkanQueue() {};
		inline uint32_t GetFamilyIndex() const { return family_index_; };
		inline VkQueue  GetQueueHandle() const { return queue_; };

		void Submit(const QueueSubmitDesc& desc);
	private:
		VkQueue queue_;
		uint32_t family_index_;
		VulkanDevice* device_;
	};
}
