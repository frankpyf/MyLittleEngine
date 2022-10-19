#include "mlepch.h"
#include "VulkanFrameResource.h"
#include "VulkanRHI.h"
#include "VulkanDevice.h"

namespace rhi {
	VulkanFrameResource::VulkanFrameResource(VulkanDevice* in_device)
		:device_(in_device)
	{
		CreateCommandPool();
		CreateSyncObjects();
	}

	VulkanFrameResource::~VulkanFrameResource()
	{
		DestroyCommandPool();
		DestroySyncObjects();
	}

	void VulkanFrameResource::ResetCommandPool()
	{
		vkResetCommandPool(device_->GetDeviceHandle(), command_pool_, VK_COMMAND_POOL_RESET_RELEASE_RESOURCES_BIT);
	}
	
	void VulkanFrameResource::AllocCommandBuffer()
	{
		VkCommandBufferAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandPool = command_pool_;
		alloc_info.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(device_->GetDeviceHandle(), &alloc_info, &command_buffer_) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to allocate command buffers!");
		};
	}

	void VulkanFrameResource::CreateSyncObjects()
	{
		VkSemaphoreCreateInfo semaphoreInfo{};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo{};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
		if (vkCreateSemaphore(device_->GetDeviceHandle(), &semaphoreInfo, nullptr, &image_acquired_semaphore_) != VK_SUCCESS ||
			vkCreateSemaphore(device_->GetDeviceHandle(), &semaphoreInfo, nullptr, &render_finished_semaphore_) != VK_SUCCESS ||
			vkCreateFence(device_->GetDeviceHandle(), &fenceInfo, nullptr, &frame_in_flight_) != VK_SUCCESS) {
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}

	void VulkanFrameResource::DestroySyncObjects()
	{
		vkDestroySemaphore(device_->GetDeviceHandle(), render_finished_semaphore_, nullptr);
		vkDestroySemaphore(device_->GetDeviceHandle(), image_acquired_semaphore_, nullptr);
		vkDestroyFence(device_->GetDeviceHandle(), frame_in_flight_, nullptr);
	}

	void VulkanFrameResource::CreateCommandPool()
	{
		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = device_->GetGfxQueue()->GetFamilyIndex();
		poolInfo.flags =
			VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

		if (vkCreateCommandPool(device_->GetDeviceHandle(), &poolInfo, nullptr, &command_pool_) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create command pool!");
		}
	}

	void VulkanFrameResource::DestroyCommandPool()
	{
		vkDestroyCommandPool(device_->GetDeviceHandle(), command_pool_, nullptr);
	}

	VulkanFrameResourceManager::VulkanFrameResourceManager(VulkanRHI& in_rhi)
		:rhi_(in_rhi), active_frame_(nullptr)
	{

	}
	
	VulkanFrameResourceManager::~VulkanFrameResourceManager()
	{

	}

	void VulkanFrameResourceManager::CreateFrames()
	{
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VulkanFrameResource* frame = new VulkanFrameResource(rhi_.GetDevice());
			frame->AllocCommandBuffer();
			frame_resources_.emplace_back(frame);
		}
		MLE_CORE_INFO("Created {0} frame resource(s)", MAX_FRAMES_IN_FLIGHT);
	}

	void VulkanFrameResourceManager::DestroyFrames()
	{
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			delete frame_resources_[i];
		}
		std::vector<VulkanFrameResource*>().swap(frame_resources_);
	}

	VulkanFrameResource* VulkanFrameResourceManager::BeginFrame()
	{
		active_frame_ = frame_resources_[current_frame_index_];
		vkWaitForFences(rhi_.GetDevice()->GetDeviceHandle(),1 , &active_frame_->GetFence(), VK_TRUE, UINT64_MAX);
		vkResetFences(rhi_.GetDevice()->GetDeviceHandle(), 1, &active_frame_->GetFence());
		current_frame_index_ = (current_frame_index_ + 1) % MAX_FRAMES_IN_FLIGHT;
		return active_frame_;
	}
}