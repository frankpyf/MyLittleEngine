#include "mlepch.h"
#include <Runtime/Core/Base/Application.h>
#include <Runtime/Core/Base/EntryPoint.h>

#include "EditorLayer.h"

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