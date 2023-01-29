#include "mlepch.h"
#include "EditorCamera.h"
#include "Runtime/Function/Input/InputSystem.h"
#include "Runtime/Function/Input/KeyCode.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>

using namespace editor;

EditorCamera::EditorCamera(float verticalFOV, float nearClip, float farClip)
	: vertical_fov(verticalFOV), near_clip_(nearClip), far_clip_(farClip)
{
	position_ = glm::vec3(0, 0, 6);

	RecalculateView();
}

bool EditorCamera::OnUpdate(float ts)
{
	glm::vec2 mousePos = engine::InputSystem::GetMousePosition();
	glm::vec2 delta = (mousePos - last_mouse_position_) * 0.002f;
	last_mouse_position_ = mousePos;

	if (!engine::InputSystem::IsMouseButtonDown(engine::MouseButton::Right))
	{
		engine::InputSystem::SetCursorMode(engine::CursorMode::Normal);
		return false;
	}

	engine::InputSystem::SetCursorMode(engine::CursorMode::Locked);

	bool moved = false;

	constexpr glm::vec3 upDirection(0.0f, 1.0f, 0.0f);
	glm::vec3 rightDirection = glm::cross(forward_direction_, upDirection);

	float speed = 5.0f;

	// Movement
	if (engine::InputSystem::IsKeyDown(engine::KeyCode::W))
	{
		position_ += forward_direction_ * speed * ts;
		moved = true;
	}
	else if (engine::InputSystem::IsKeyDown(engine::KeyCode::S))
	{
		position_ -= forward_direction_ * speed * ts;
		moved = true;
	}
	if (engine::InputSystem::IsKeyDown(engine::KeyCode::A))
	{
		position_ -= rightDirection * speed * ts;
		moved = true;
	}
	else if (engine::InputSystem::IsKeyDown(engine::KeyCode::D))
	{
		position_ += rightDirection * speed * ts;
		moved = true;
	}
	if (engine::InputSystem::IsKeyDown(engine::KeyCode::Q))
	{
		position_ -= upDirection * speed * ts;
		moved = true;
	}
	else if (engine::InputSystem::IsKeyDown(engine::KeyCode::E))
	{
		position_ += upDirection * speed * ts;
		moved = true;
	}

	// Rotation
	if (delta.x != 0.0f || delta.y != 0.0f)
	{
		float pitchDelta = delta.y * GetRotationSpeed();
		float yawDelta = delta.x * GetRotationSpeed();

		glm::quat q = glm::normalize(glm::cross(glm::angleAxis(-pitchDelta, rightDirection),
			glm::angleAxis(-yawDelta, glm::vec3(0.f, 1.0f, 0.0f))));
		forward_direction_ = glm::rotate(q, forward_direction_);

		moved = true;
	}

	if (moved)
	{
		RecalculateView();
	}

	return moved;
}

void EditorCamera::OnResize(uint32_t width, uint32_t height)
{
	if (width == viewport_width_ && height == viewport_height_)
		return;

	viewport_width_ = width;
	viewport_height_ = height;

	RecalculateProjection();
}

float EditorCamera::GetRotationSpeed()
{
	return 0.3f;
}

void EditorCamera::RecalculateProjection()
{
	projection_ = glm::perspectiveFov(glm::radians(vertical_fov), (float)viewport_width_, (float)viewport_height_, near_clip_, far_clip_);
	inverse_projection_ = glm::inverse(projection_);
}

void EditorCamera::RecalculateView()
{
	view_ = glm::lookAt(position_, position_ + forward_direction_, glm::vec3(0, 1, 0));
	inverse_view_ = glm::inverse(view_);
}
