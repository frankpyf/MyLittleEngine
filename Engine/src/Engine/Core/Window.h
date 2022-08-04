#pragma once
struct GLFWwindow;

namespace Engine {
	struct WindowProps
	{
		std::string Title;
		uint32_t Width;
		uint32_t Height;

		WindowProps(const std::string& title = "Hazel Engine",
			uint32_t width = 1600,
			uint32_t height = 900)
			: Title(title), Width(width), Height(height)
		{
		}
	};

	// Interface representing a desktop system based Window
	class Window
	{
	public:
		//using EventCallbackFn = std::function<void(Event&)>;
		Window(const WindowProps& props);
		virtual ~Window();

		void OnUpdate();

		uint32_t getWidth() const	{ return m_Props.Width; };
		uint32_t getHeight() const	{ return m_Props.Height; };

		// Window attributes
		/*virtual void seteventcallback(const eventcallbackfn& callback) = 0;
		virtual void setvsync(bool enabled) = 0;
		virtual bool isvsync() const = 0;*/

		virtual void* getNativeWindow() const { return m_Window; };
	private:
		//data
		GLFWwindow* m_Window;
		WindowProps m_Props;
		//function
		void Init(const WindowProps& props);
		void Shutdown();
	};
}

