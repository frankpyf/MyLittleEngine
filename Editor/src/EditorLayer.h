#pragma once
#include "MLE.h"
#include "EditorCamera.h"
#include "SceneHierachyPanel.h"
struct AtmosphereParameter
{
	glm::vec3 sun_light_direction{0.0, 1.0, 1.0};
	float sun_light_intensity = 31.4;
	glm::vec3 sun_light_color{ 1.0, 1.0,1.0 };
	float sea_level = 0;
	glm::vec3 planet_center{ 0, 0, -100.0 };
	float planet_radius = 6360000;
	float atmosphere_height = 60000;
	float sun_disk_angle = 1;
	float rayleigh_scattering_scale = 1;
	float rayleigh_scattering_scalar_height = 8000;
	float mie_scattering_scale = 1;
	float mie_anisotropy = 0.8;
	float mie_scattering_scalar_height = 1200;
	float ozone_absorption_scale = 1;
	float ozone_level_center_height = 25000;
	float ozone_level_width = 15000;
};
struct CameraUbo {
	glm::mat4 inverse_view;
	glm::mat4 inverse_proj;
	glm::vec3 position;
};

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
		rhi::BufferRef camera_ubo_[renderer::FrameResourceMngr::MAX_FRAMES_IN_FLIGHT];

		std::shared_ptr<engine::Scene> editor_scene_;
		engine::Entity square_entity_;
		engine::Entity light_entity_;

		SceneHierachyPanel scene_panel_;

		// TEMP
		std::unique_ptr<rhi::DescriptorAllocator> desc_allocator_;
		std::unique_ptr<rhi::DescriptorSetLayoutCache> layout_cache_;

		rhi::DescriptorSet*			global_set_[renderer::FrameResourceMngr::MAX_FRAMES_IN_FLIGHT];
		rhi::DescriptorSetLayout*	global_set_layout_;
		// used by sky pass
		/*rhi::DescriptorSetLayout* texture_set_layout_;

		rhi::DescriptorSetLayout* buffer_set_layout_;*/

		// rhi::DescriptorSet* texture_set_[renderer::FrameResourceMngr::MAX_FRAMES_IN_FLIGHT];
		// rhi::DescriptorSet* buffer_set_[renderer::FrameResourceMngr::MAX_FRAMES_IN_FLIGHT];

		// rhi::PipelineLayout* pipeline_layout_;
		rhi::PipelineLayout* atmosphere_pipeline_layout_;
		// rhi::PipelineLayout* combine_pipeline_layout_;

		rhi::BufferRef vb_;
		rhi::BufferRef indicies_;
		glm::vec2 viewport_size_ = { 800.0f, 800.0f };

		AtmosphereParameter param_;
		rhi::BufferRef param_ubo_[renderer::FrameResourceMngr::MAX_FRAMES_IN_FLIGHT];

		uint8_t frame_index_;
	};
}