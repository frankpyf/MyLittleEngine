#pragma once
#include "MLE.h"

namespace editor{
	class EditorLayer: public engine::Layer
	{
	public:
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUIRender() override;
		virtual void OnUpdate(float delta_time) override;
 	private:
		std::shared_ptr<rhi::RHITexture2D> back_buffer_;
		glm::vec2 viewport_size_ = { 800.0f, 800.0f };
	};
}