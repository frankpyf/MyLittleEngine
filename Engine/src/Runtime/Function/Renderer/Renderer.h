#pragma once
#include "Runtime/Function/RHI/RHICommands.h"

namespace renderer {
	class Renderer
	{
	public:
		Renderer();
		virtual ~Renderer();

		Renderer(const Renderer&) = delete;
		Renderer& operator=(const Renderer&) = delete;

		void Begin();
		void Tick(float time_step);
		void End();

		void Init();

		void Shutdown();
	private:

		bool is_frame_started_{ false };
	};
}

