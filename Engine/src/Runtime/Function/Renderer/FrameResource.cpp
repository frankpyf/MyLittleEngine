#include "mlepch.h"
#include "FrameResource.h"
#include "Runtime/Function/RHI/RHI.h"

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
			for (auto& rt : frame_[index].render_targets)
			{
				if (rt)
					delete rt;
			}
			frame_[index].render_targets.clear();

			rhi.RHIDestroyFence(frame_[index].in_flight_fence);
			rhi.RHIDestroySemaphore(frame_[index].image_acquired_semaphore);
			rhi.RHIDestroySemaphore(frame_[index].render_finished_semaphore);
		}
	}

	FrameResource& FrameResourceMngr::BeginFrame()
	{
		rhi::RHI& rhi = rhi::RHI::GetRHIInstance();
		current_frame = (current_frame + 1) % MAX_FRAMES_IN_FLIGHT;
		rhi::Fence* fences[1] = { frame_[current_frame].in_flight_fence };
		rhi.RHIWaitForFences(fences, 1);

		rhi.AcquireNextImage(frame_[current_frame].image_acquired_semaphore);

		frame_[current_frame].command_buffer->Begin();
		// Since we need to recreate rts each frame, so ...
		for (auto& rt : frame_[current_frame].render_targets)
		{
			delete rt;
		}
		// reserve the capability though
		frame_[current_frame].render_targets.clear();

		return frame_[current_frame];
	}

	FrameResource& FrameResourceMngr::EndFrame()
	{
		frame_[current_frame].command_buffer->End();
		return frame_[current_frame];
	}
}