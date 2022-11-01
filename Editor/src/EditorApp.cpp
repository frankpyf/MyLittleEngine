#include "mlepch.h"
#include <Runtime/Core/Base/Application.h>
#include <Runtime/Core/Base/EntryPoint.h>

#include "EditorLayer.h"

class ExampleLayer : public engine::Layer
{
public:
	virtual void OnUIRender() override
	{
		ImGui::Begin("Hello");
		ImGui::Button("Button");
		ImGui::End();

		ImGui::ShowDemoWindow();  
	}
};

engine::Application* engine::CreateApplication(int argc, char** argv)
{
	engine::ApplicationSpecification spec;
	spec.name = "Editor";
	spec.width = 1920;
	spec.height = 1080;

	engine::Application* app = new engine::Application(spec);
	app->PushLayer<editor::EditorLayer>();
	return app;
}