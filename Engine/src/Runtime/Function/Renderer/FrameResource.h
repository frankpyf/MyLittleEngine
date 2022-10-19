#pragma once
namespace graphics {
	class RHI;
}
namespace renderer {

	class FrameResource
	{
	public:
		FrameResource(graphics::RHI& in_rhi);
		virtual ~FrameResource() = default;
		virtual void ResetCommandPool() = 0;
		virtual void AllocCommandBuffer() = 0;
		virtual void* GetCommandBuffer() = 0;
	};	
}

