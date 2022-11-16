#pragma once
#include "Runtime/Core/Base/Singleton.h"
#include "Runtime/Function/RHI/RHICommands.h"
#include "FrameResource.h"

namespace rhi{
	class RHIVertexBuffer;
}

namespace resource {
	struct Vertex;
}

namespace renderer {
	class Renderer : public engine::Singleton<Renderer>
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

		std::shared_ptr<rhi::RHIVertexBuffer> LoadModel(const std::vector<resource::Vertex> in_vertices);
	private:
		FrameResourceMngr frames_manager_;
		bool is_frame_started_{ false };
	};
}

