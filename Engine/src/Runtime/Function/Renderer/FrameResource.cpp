#include "mlepch.h"
#include "FrameResource.h"
#include "Runtime/Function/RHI/RHI.h"
#include "Runtime/Resource/Vertex.h"

namespace renderer {
	void FrameResourceMngr::CreateFrames()
	{
		rhi::RHI& rhi = rhi::RHI::GetRHIInstance();
		for (uint8_t index = 0; index < MAX_FRAMES_IN_FLIGHT; index++)
		{
			frame_[index].command_buffer = rhi.RHICreateCommandBuffer();
			frame_[index].in_flight_fence = rhi.RHICreateFence();
			frame_[index].image_acquired_semaphore = rhi.RHICreateSemaphore();
			frame_[index].render_finished_semaphore = rhi.RHICreateSemaphore();
		}
	}

	void FrameResourceMngr::DestroyFrames()
	{
		rhi::RHI& rhi = rhi::RHI::GetRHIInstance();
		for (uint8_t index = 0; index < MAX_FRAMES_IN_FLIGHT; index++)
		{
			if (frame_[index].command_buffer)
				delete frame_[index].command_buffer;

			rhi.RHIDestroyFence(frame_[index].in_flight_fence);
			rhi.RHIDestroySemaphore(frame_[index].image_acquired_semaphore);
			rhi.RHIDestroySemaphore(frame_[index].render_finished_semaphore);

			for (auto texture : frame_[index].texture_dump)
			{
				rhi.RHIFreeTexture(*texture);
			}
			frame_[index].texture_dump.clear();
			frame_[index].render_target_dump.clear();
		}
	}

	FrameResource& FrameResourceMngr::BeginFrame()
	{
		rhi::RHI& rhi = rhi::RHI::GetRHIInstance();
		// TEMP: will come up with a better gc strategy
		for (auto& frame : frame_)
		{
			if (rhi.RHIIsFenceReady(frame.in_flight_fence))
			{
				for (auto texture : frame.texture_dump)
				{
					rhi.RHIFreeTexture(*texture);
				}
				frame.texture_dump.clear();
				frame.render_target_dump.clear();
			}
		}
		current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
		rhi::Fence* fences[1] = { frame_[current_frame].in_flight_fence };
		rhi.RHIWaitForFences(fences, 1);

		rhi.AcquireNextImage(frame_[current_frame].image_acquired_semaphore);

		frame_[current_frame].command_buffer->Begin();

		return frame_[current_frame];
	}

	FrameResource& FrameResourceMngr::EndFrame()
	{
		frame_[current_frame].command_buffer->End();
		return frame_[current_frame];
	}

	void FrameResourceMngr::Clean()
	{
		for (auto& frame : frame_)
		{
			
		}
	}
}