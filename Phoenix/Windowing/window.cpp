#include <Windowing/Window.hpp>

#include <assert.h>

Window::Window( const char* title, int width, int height )
{
	bool initSDL = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) == 0;
	if(!initSDL)
	{
		assert( initSDL && "Error, unable to init SDL" );
		return;
	}

	mWindow = SDL_CreateWindow(
		title,
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		width, height,
		SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN
	);

	if (!mWindow)
	{
		assert( 0 && "Failed it make window" );
		SDL_Quit();
		return;
	}
	SDL_ShowWindow( mWindow );

	mWidth = width;
	mHeight = height;

	SDL_VERSION( &mWindowInfo.version );
	bool sucsess = SDL_GetWindowWMInfo( mWindow, &mWindowInfo );
	assert( sucsess && "Error, unable to get window info" );
	mOpen = true;
	mRenderable = true;
}

Window::~Window( )
{
	SDL_DestroyWindow( mWindow );
}

void Window::AddEventCallback( SDL_EventType type, std::function<void( SDL_Event&, void* ref )> callback, void* ref )
{
	mEventCallbacks[type].push_back( { callback, ref } );
}

void Window::Poll( )
{
	SDL_Event event;
	while ( SDL_PollEvent( &event ) > 0 )
	{
		switch ( event.type )
		{
		case SDL_QUIT:
			mOpen = false;
			break;
		case SDL_WINDOWEVENT:
			switch ( event.window.event )
			{
				case SDL_WINDOWEVENT_SIZE_CHANGED:
				{
					mWidth = event.window.data1;
					mHeight = event.window.data2;
					break;
				}
				case SDL_WINDOWEVENT_MINIMIZED:
				{
					mRenderable = false;
					break;
				}
				case SDL_WINDOWEVENT_MAXIMIZED:
				{
					mRenderable = true;
					break;
				}
			}
			break;
		}
		for ( auto callback : mEventCallbacks[static_cast<SDL_EventType>(event.type)] )
		{
			callback.functionPtr( event, callback.refrencePtr );
		}
	}
}

bool Window::IsRenderable()
{
	return mRenderable;
}
