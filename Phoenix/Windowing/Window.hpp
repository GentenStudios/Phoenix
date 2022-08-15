#pragma once

#include <SDL.h>
#include <SDL_syswm.h>

#include <functional>
#include <map>
#include <vector>

class Window
{
private:
	struct EventCallback
	{
		std::function<void(SDL_Event&, void* ref)> functionPtr;
		void*                                      referencePtr;
	};

public:
	Window(const char* title, int width, int height);
	~Window();

	bool IsOpen() const;
	void Close();
	void Poll();

	void AddEventCallback(SDL_EventType type, std::function<void(SDL_Event&, void* ref)> callback, void* ref);

	bool IsRenderable();

	void CaptureMouse(bool capture);
	void GrabMouse(bool grab);

	uint32_t    GetWidth() const;
	uint32_t    GetHeight() const;
	SDL_Window* GetWindow() const;

private:
	SDL_Window*   m_window;
	SDL_SysWMinfo m_windowInfo;

	uint32_t m_width;
	uint32_t m_height;

	bool m_open;
	bool m_renderable;

	bool m_mouseCapture;
	bool m_mouseGrab;

	std::map<SDL_EventType, std::vector<EventCallback>> mEventCallbacks;
};
