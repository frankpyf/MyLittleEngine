#pragma once

namespace renderer {
	class Camera
	{
	public:
		Camera() = default;
		Camera(const glm::mat4& projection)
			: projection_(projection) {}

		virtual ~Camera() = default;

		const glm::mat4& GetProjection() const { return projection_; }
	protected:
		glm::mat4 projection_{ 1.0f };
	};
}