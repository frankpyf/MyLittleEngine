#pragma once
#include "Runtime/Core/Base/Application.h"
#include "Runtime/Core/Base/Log.h"
#ifdef MLE_PLATFORM_WINDOWS

extern engine::Application* engine::CreateApplication(int argc, char** argv);
bool g_ApplicationRunning = true;

namespace engine {

	int Main(int argc, char** argv)
	{
		while (g_ApplicationRunning)
		{
			engine::Log::Init();
			MLE_CORE_INFO("Engine running");
			engine::Application* app = engine::CreateApplication(argc, argv);
			app->Run();
			delete app;
		}
		return 0;
	}

}

#ifdef MLE_DIST

#include <Windows.h>

int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
{
	return engine::Main(__argc, __argv);
}

#else

int main(int argc, char** argv)
{
	return engine::Main(argc, argv);
}

#endif // MLE_DIST

#endif // MLE_PLATFORM_WINDOWS
