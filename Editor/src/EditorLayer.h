#pragma once
#include "MLE.h"

namespace editor{
	class EditorLayer: public engine::Layer
	{
	public:
		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUIRender() override;
	private:
	};
}