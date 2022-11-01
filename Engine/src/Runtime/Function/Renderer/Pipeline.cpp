#include "mlepch.h"
#include "Pipeline.h"
#include "Runtime/Function/RHI/RHI.h"
#include "Runtime/Platform/Vulkan/VulkanPipeline.h"

namespace renderer {
	Pipeline* Pipeline::Create(const char* vert_path,
								const char* frag_path,
								const PipelineDesc& desc)
	{
		rhi::RHI& rhi = rhi::RHI::GetRHIInstance();
		switch (rhi::RHI::GetAPI())
		{
		case rhi::RHI::GfxAPI::None: return 0;
		case rhi::RHI::GfxAPI::Vulkan: return rhi.RHICreatePipeline(vert_path, frag_path, desc);
		}
	}
}