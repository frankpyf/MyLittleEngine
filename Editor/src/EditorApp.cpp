#include "mlepch.h"
#include <Runtime/Core/Base/Application.h>
#include <Runtime/Core/Base/EntryPoint.h>

#include "Runtime/Image.h"

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

	engine::Application* app = new engine::Application(spec);
	//app->PushLayer<ExampleLayer>();
	//app->SetMenubarCallback([app]()
	//{
	//	if (ImGui::BeginMenu("File"))
	//	{
	//		if (ImGui::MenuItem("Open File"))
	//		{
	//			//*****open file callback here********
	//		}
	//		if (ImGui::MenuItem("Exit"))
	//		{
	//			app->Close();
	//		}
	//		ImGui::EndMenu();
	//	}
	//});
	return app;
}