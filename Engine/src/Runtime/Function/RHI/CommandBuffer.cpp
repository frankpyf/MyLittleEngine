#include "mlepch.h"
#include "CommandBuffer.h"
#include "RHI.h"
#include "Runtime/Platform/Vulkan/VulkanCommandBuffer.h"

namespace rhi {
	CommandBuffer* CommandBuffer::Create()
	{
		RHI& rhi = RHI::GetRHIInstance();
		return rhi.RHICreateCommandBuffer();
	}
}