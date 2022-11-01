#pragma once

namespace engine {

	class Layer
	{
	public:
		virtual ~Layer() = default;

		virtual void OnAttach() {}
		virtual void OnDetach() {}

		virtual void OnUIRender() {}

		virtual void OnUpdate() {}
	};

}