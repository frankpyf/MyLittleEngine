#pragma once
#include <vulkan/vulkan.h>
#include "VulkanQueue.h"

namespace rhi {
	class VulkanDevice
	{
		friend class VulkanRHI;
	public:
		
		VulkanDevice(VulkanRHI* in_rhi, VkPhysicalDevice gpu);
		~VulkanDevice();

		void Destroy();

		VulkanDevice(const VulkanDevice&) = delete;
		VulkanDevice& operator=(const VulkanDevice&) = delete;
		VulkanDevice(VulkanDevice&&) = delete;
		VulkanDevice& operator=(VulkanDevice&&) = delete;

		bool QueryGPU();
		void InitGPU();
		void CreateLogicalDevice();

		void CreateCommandPool();
		void CreateDescriptorPool();

		VkCommandBuffer BeginSingleTimeCommands();
		void			EndSingleTimeCommands(VkCommandBuffer command_buffer);

		inline VkDevice			GetDeviceHandle()	const { return device_; };
		inline VkPhysicalDevice GetPhysicalHandle() const { return gpu_; };
		inline VulkanQueue*		GetGfxQueue()		{ return graphics_queue_; };
		inline VulkanQueue*		GetComputeQueue()	{ return compute_queue_; };
		inline VulkanQueue*		GetPresentQueue()	{ return present_queue_; };
		inline VkCommandPool	GetCommandPool()	{ return command_pool_; };
		inline VkDescriptorPool	GetDescriptorPool()	{ return descriptor_pool_; }

		inline const VkPhysicalDeviceFeatures& GetPhysicalFeatures() { return features_; };
		inline const VkPhysicalDeviceProperties& GetDeviceProperties() { return gpu_properties_; };

		void SetupPresentQueue(VkSurfaceKHR in_surface);

	private:
		//Physical Device, aka gpu
		VkPhysicalDevice gpu_;
		VkPhysicalDeviceProperties gpu_properties_{};
		VkPhysicalDeviceFeatures features_{};
		// Logical Device
		VkDevice         device_;

		// other stuff

		VkCommandPool    command_pool_;
		VkDescriptorPool descriptor_pool_;

		// Queue
		VulkanQueue* graphics_queue_;
		VulkanQueue* compute_queue_;
		VulkanQueue* transfer_queue_;
		VulkanQueue* present_queue_;
		std::vector<VkQueueFamilyProperties> queue_family_props_;

		VulkanRHI* rhi_ = nullptr;
		
	};
}