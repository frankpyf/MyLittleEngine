#include "mlepch.h"
#include "RHIResource.h"
#include "RHI.h"

namespace rhi {
	std::shared_ptr<RHITexture2D> RHITexture2D::Create(Descriptor desc)
	{
		rhi::RHI& rhi = rhi::RHI::GetRHIInstance();
		return rhi.RHICreateTexture2D(desc);
	}
}