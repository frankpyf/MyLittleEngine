#include "mlepch.h"
#include <Engine/Core/Application.h>
#include <Engine/Core/EntryPoint.h>

#include "Engine/Image.h"

class ExampleLayer : public Engine::Layer
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

Engine::Application* Engine::CreateApplication(int argc, char** argv)
{
	Engine::ApplicationSpecification spec;
	spec.Name = "Editor";

	Engine::Application* app = new Engine::Application(spec);
	app->PushLayer<ExampleLayer>();
	app->SetMenubarCallback([app]()
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Exit"))
			{
				app->Close();
			}
			ImGui::EndMenu();
		}
	});
	return app;
}