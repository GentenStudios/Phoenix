#include <Windowing/Window.hpp>

//#include <SDL_vulkan.h>

#include <cassert>

Window::Window(const char* title, int width, int height)
{
	m_mouseCapture = false;
	m_mouseGrab    = false;

	const bool initSDL = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0;
	if (!initSDL)
	{
		assert(initSDL && "Error, unable to init SDL");
		return;
	}

	m_window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height,
	                           SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_MOUSE_CAPTURE);

	if (!m_window)
	{
		assert(0 && "Failed it make window");
		SDL_Quit();
		return;
	}

	SDL_ShowWindow(m_window);

	m_width  = width;
	m_height = height;

	SDL_VERSION(&m_windowInfo.version);

	const bool success = SDL_GetWindowWMInfo(m_window, &m_windowInfo);
	assert(success && "Error, unable to get window info");

	m_open       = true;
	m_renderable = true;
}

Window::~Window() { SDL_DestroyWindow(m_window); }

bool Window::IsOpen() const { return m_open; }

void Window::Close() { m_open = false; }

uint32_t Window::GetWidth() const { return m_width; }

uint32_t Window::GetHeight() const { return m_height; }

SDL_Window* Window::GetWindow() const { return m_window; }

void Window::AddEventCallback(SDL_EventType type, std::function<void(SDL_Event&, void* ref)> callback, void* ref)
{
	mEventCallbacks[type].push_back({callback, ref});
}

void Window::Poll()
{
	SDL_Event event;
	while (SDL_PollEvent(&event) > 0)
	{
		switch (event.type)
		{
		case SDL_QUIT:
			m_open = false;
			break;
		case SDL_WINDOWEVENT:
			switch (event.window.event)
			{
			case SDL_WINDOWEVENT_SIZE_CHANGED:
			{
				m_width  = event.window.data1;
				m_height = event.window.data2;
				break;
			}
			case SDL_WINDOWEVENT_MINIMIZED:
			{
				m_renderable = false;
				break;
			}
			case SDL_WINDOWEVENT_MAXIMIZED:
			{
				m_renderable = true;
				break;
			}
			}
			break;
		default:
			break;
		}
		for (auto callback : mEventCallbacks[static_cast<SDL_EventType>(event.type)])
		{
			callback.functionPtr(event, callback.referencePtr);
		}
	}
}

bool Window::IsRenderable() { return m_renderable; }

void Window::CaptureMouse(bool capture)
{
	if (capture != m_mouseCapture)
	{
		//SDL_CaptureMouse((SDL_bool) capture);
		m_mouseCapture = capture;
	}
}

void Window::GrabMouse(bool grab)
{
	if (grab != m_mouseGrab)
	{
		//SDL_SetWindowGrab(mWindow, (SDL_bool) grab);
		SDL_SetRelativeMouseMode((SDL_bool) grab);
		m_mouseGrab = grab;
	}
}
