#pragma once
#include <vulkan/vulkan.h>

namespace engine {
	class VulkanQueue;

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

		inline VkDevice			GetDeviceHandle()	const { return device_; };
		inline VkPhysicalDevice GetPhysicalHandle() const { return gpu_; };
		inline VulkanQueue*		GetGfxQueue()		{ return graphics_queue_; };
		inline VulkanQueue*		GetComputeQueue()	{ return compute_queue_; };
		inline VulkanQueue*		GetPresentQueue()	{ return present_queue_; };
		inline VkCommandPool	GetCommandPool()	{ return command_pool_; }

		inline const VkPhysicalDeviceFeatures& const GetPhysicalFeatures() { return features_; };
		inline const VkPhysicalDeviceProperties& const GetDeviceProperties() { return gpu_properties_; };

		uint32_t FindMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties);
		VkFormat FindSupportedFormat(
			const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

		void CreateImageWithInfo(
			const VkImageCreateInfo& image_info,
			VkMemoryPropertyFlags properties,
			VkImage& image,
			VkDeviceMemory& image_memory);

		void SetupPresentQueue(VkSurfaceKHR in_surface);

	private:
		//Physical Device, aka gpu
		VkPhysicalDevice gpu_;
		VkPhysicalDeviceProperties gpu_properties_;
		VkPhysicalDeviceFeatures features_;
		// Logical Device
		VkDevice         device_;
		// other stuff
		VkCommandPool command_pool_;

		// Queue
		VulkanQueue* graphics_queue_;
		VulkanQueue* compute_queue_;
		VulkanQueue* transfer_queue_;
		VulkanQueue* present_queue_;
		std::vector<VkQueueFamilyProperties> queue_family_props_;

		VulkanRHI* rhi_ = nullptr;
		
	};
}