#include "InputHandler.hpp"

#include <Windowing/Window.hpp>

#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_vulkan.h>

void MouseKeyEvent(SDL_Event& event, void* ref)
{
	phx::InputHandler* inputHander = reinterpret_cast<phx::InputHandler*>(ref);
	
	bool pressed = event.button.state == SDL_PRESSED;
	switch (event.button.button)
	{
	case SDL_BUTTON_LEFT:
		inputHander->MouseEvent(0, pressed);
		break;
	case SDL_BUTTON_RIGHT:
		inputHander->MouseEvent(1, pressed);
		break;
	case SDL_BUTTON_MIDDLE:
		inputHander->MouseEvent(2, pressed);
		break;
	}

}

void WindowKeyEvent(SDL_Event& event, void* ref)
{
	phx::InputHandler* inputHander = reinterpret_cast<phx::InputHandler*>(ref);
	int key = event.key.keysym.scancode;
	inputHander->KeyEvent(key, event.type == SDL_KEYDOWN);
}


phx::InputHandler::InputHandler(Window* window) : mWindow(window)
{
	window->AddEventCallback(SDL_MOUSEBUTTONDOWN, MouseKeyEvent, this);
	window->AddEventCallback(SDL_MOUSEBUTTONUP, MouseKeyEvent, this);

	window->AddEventCallback(SDL_KEYDOWN, WindowKeyEvent, this);
	window->AddEventCallback(SDL_KEYUP, WindowKeyEvent, this);
	for (int i = 0; i < 256; ++i)
	{
		mKeys[i] = false;
	}
}

phx::InputHandler::~InputHandler()
{

}

void phx::InputHandler::KeyEvent(int key, bool pressed)
{
	if (key >= 256)
		return;
	mKeys[key] = pressed;
}

void phx::InputHandler::MouseEvent(int mouseKey, bool pressed)
{
	if (mouseKey >= 3)
		return;
	mMouseKeys[mouseKey] = pressed;
}

bool phx::InputHandler::IsPressed(int key)
{
	if (key >= 256)
		return false;
	return mKeys[key];
}

bool phx::InputHandler::IsMousePressed(int key)
{ 
	if (key >= 3)
		return false;
	return mMouseKeys[key];
}
