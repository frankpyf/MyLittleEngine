#pragma once
#include "RHI.h"
#include "Runtime/Function/RHI/CommandBuffer.h"

namespace rhi {
	class RHICommands
	{
	public:
		static void Init()
		{
			rhi_.Init();
		};
		
		static void GfxQueueSubmit(const rhi::QueueSubmitDesc& desc)
		{
			rhi_.GfxQueueSubmit(desc);
		}

		static void TransferQueueSubmit(const rhi::QueueSubmitDesc& desc)
		{
			rhi_.TransferQueueSubmit(desc);
		}
		
		static void Present(rhi::Semaphore** semaphores, uint32_t semaphore_count)
		{
			rhi_.Present(semaphores, semaphore_count);
		}

		static void* GetRHIInstance()
		{
			return rhi_.GetNativeInstance();
		}
		static void* GetGfxQueue()
		{
			return rhi_.GetNativeGraphicsQueue();
		}
		static uint32_t GetGfxQueueFamilyIndex()
		{
			return rhi_.GetGfxQueueFamily();
		}
		static void* GetPhysicalDevice()
		{
			return rhi_.GetNativePhysicalDevice();
		}
		static void Shutdown()
		{
			rhi_.Shutdown();
		}

	private:
		static RHI& rhi_;
	};
}

