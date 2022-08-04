#pragma once

#ifdef MLE_PLATFORM_WINDOWS

extern Engine::Application* Engine::CreateApplication(int argc, char** argv);
bool g_ApplicationRunning = true;

namespace Engine {

	int Main(int argc, char** argv)
	{
		while (g_ApplicationRunning)
		{
			Engine::Application* app = Engine::CreateApplication(argc, argv);
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
	return Engine::Main(__argc, __argv);
}

#else

int main(int argc, char** argv)
{
	return Engine::Main(argc, argv);
}

#endif // MLE_DIST

#endif // MLE_PLATFORM_WINDOWS
