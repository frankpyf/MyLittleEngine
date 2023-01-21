#pragma once
#include "MLE.h"

namespace editor
{
	class EditorCamera : public renderer::Camera
	{
	public:
		EditorCamera() = default;
		EditorCamera(float verticalFOV, float nearClip, float farClip);

		bool OnUpdate(float ts);
		void OnResize(uint32_t width, uint32_t height);

		const glm::mat4& GetProjection() const { return projection_; }
		const glm::mat4& GetInverseProjection() const { return inverse_projection_; }
		const glm::mat4& GetView() const { return view_; }
		const glm::mat4& GetInverseView() const { return inverse_view_; }

		const glm::vec3& GetPosition() const { return position_; }
		const glm::vec3& GetDirection() const { return forward_direction_; }

		const std::vector<glm::vec3>& GetRayDirections() const { return ray_directions_; }

		float GetRotationSpeed();
	private:
		void RecalculateProjection();
		void RecalculateView();
		void RecalculateRayDirections();
	private:
		glm::mat4 view_{ 1.0f };
		glm::mat4 inverse_projection_{ 1.0f };
		glm::mat4 inverse_view_{ 1.0f };

		float vertical_fov = 45.0f;
		float near_clip_ = 0.1f;
		float far_clip_ = 100.0f;

		glm::vec3 position_{ 0.0f, 0.0f, 0.0f };
		glm::vec3 forward_direction_{ 0.0f, 0.0f, -1.0f };

		std::vector<glm::vec3> ray_directions_;

		glm::vec2 last_mouse_position_{ 0.0f, 0.0f };

		uint32_t viewport_width_ = 0, viewport_height_ = 0;
	};
} // namespace editor
