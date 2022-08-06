#pragma once

#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_vulkan.h>

#include <functional>
#include <vector>
#include <map>

class Window
{

	struct EventCallback
	{
		std::function<void( SDL_Event&, void* ref )> functionPtr;
		void* refrencePtr;
	};
public:
	Window( const char* title, int width, int height );
	~Window( );

	bool IsOpen( ) { return mOpen; }

	void Close( ) { mOpen = false; }

	uint32_t GetWidth( ) { return mWidth; }
	uint32_t GetHeight( ) { return mHeight; }

	void AddEventCallback( SDL_EventType type, std::function<void( SDL_Event&, void* ref )> callback, void* ref );

	void Poll( );

	bool IsRenderable();

	SDL_Window* GetWindow( ) { return mWindow; }

private:
	SDL_Window* mWindow;
	SDL_SysWMinfo mWindowInfo;

	uint32_t mWidth;
	uint32_t mHeight;

	bool mOpen;

	bool mRenderable;

	std::map<SDL_EventType, std::vector<EventCallback>> mEventCallbacks;
};