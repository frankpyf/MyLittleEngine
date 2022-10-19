#pragma once
#include <vulkan/vulkan.h>

namespace rhi {
	class VulkanRHI;
	class VulkanDevice;

	class VulkanFrameResource
	{		
	friend class VulkanFrameResourece;
	public:
		VulkanFrameResource(VulkanDevice* in_device);
		virtual ~VulkanFrameResource();
		void ResetCommandPool();
		void AllocCommandBuffer();
		void CreateSyncObjects();
		void DestroySyncObjects();
		inline VkCommandBuffer& GetCommandBuffer() { return command_buffer_; };
		inline VkFence& GetFence() { return frame_in_flight_; };
		inline VkSemaphore& GetImageAcquireSemaphore() { return image_acquired_semaphore_; };
		inline VkSemaphore& GetRenderFinishedSemaphore() { return render_finished_semaphore_; };
	private:
		void CreateCommandPool();
		void DestroyCommandPool();

		VulkanDevice* device_;

		VkCommandPool command_pool_ = VK_NULL_HANDLE;
		VkCommandBuffer command_buffer_ = VK_NULL_HANDLE;

		VkFence frame_in_flight_ = VK_NULL_HANDLE;
		VkSemaphore image_acquired_semaphore_ = VK_NULL_HANDLE;
		VkSemaphore render_finished_semaphore_ = VK_NULL_HANDLE;

	};

	class VulkanFrameResourceManager
	{

	public:
		static constexpr uint8_t MAX_FRAMES_IN_FLIGHT = 2;
		VulkanFrameResourceManager(VulkanRHI& in_rhi);
		~VulkanFrameResourceManager();
		void CreateFrames();
		void DestroyFrames();

		// Call this function at the beginning of every frame
		VulkanFrameResource* BeginFrame();
		VulkanFrameResource* GetActiveFrame() { return active_frame_; };
	private:
		VulkanRHI& rhi_;
		std::vector<VulkanFrameResource*> frame_resources_;
		VulkanFrameResource* active_frame_;
		int current_frame_index_ = 0;
	};
}

