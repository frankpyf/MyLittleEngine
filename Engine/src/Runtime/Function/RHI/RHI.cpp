#include "mlepch.h"
#include "RHI.h"

#include "Runtime/Platform/Vulkan/VulkanRHI.h"

namespace rhi{

    RHI::GfxAPI RHI::api_ = RHI::GfxAPI::Vulkan;

    RHI& RHI::GetRHIInstance()
    {
        switch (api_)
        {
        case RHI::GfxAPI::None:    assert(false && "Need to select a RendererAPI!"); break;
        case RHI::GfxAPI::Vulkan:  static VulkanRHI rhi; return rhi;
        }

        assert(false && "Unknown RendererAPI!");
    }
}