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
		
		static void GfxQueueSubmit(rhi::CommandBuffer* cmd_buffer)
		{
			rhi_.GfxQueueSubmit(cmd_buffer);
		}
		
		static void Present(void* semaphore)
		{
			rhi_.Present(semaphore);
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

