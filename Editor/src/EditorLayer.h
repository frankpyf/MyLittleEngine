#pragma once
#include "MLE.h"
#include "EditorCamera.h"

namespace editor{
	class EditorLayer: public engine::Layer
	{
	public:
		EditorLayer();
		virtual ~EditorLayer() = default;
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUIRender() override;
		virtual void OnUpdate(float delta_time) override;
 	private:
		std::shared_ptr<rhi::RHITexture2D> back_buffer_;

		EditorCamera editor_camera_;

		std::shared_ptr<engine::Scene> editor_scene_;
		engine::Entity square_entity_;

		// TEMP
		std::unique_ptr<rhi::DescriptorAllocator> desc_allocator_;
		std::unique_ptr<rhi::DescriptorSetLayoutCache> layout_cache_;
		rhi::DescriptorSetLayout* global_set_layout_;

		rhi::PipelineLayout* pipeline_layout_;

		rhi::ShaderModule* vert_shader_;
		rhi::ShaderModule* frag_shader_;

		rhi::BufferRef vb_;
		rhi::BufferRef indicies_;
		glm::vec2 viewport_size_ = { 800.0f, 800.0f };
	};
}