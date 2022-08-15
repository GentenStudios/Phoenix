// BSD 3-Clause License
// 
// Copyright (c) 2022, Genten Studios
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
// 
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
// 
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
// 
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "InputHandler.hpp"

#include <Windowing/Window.hpp>

#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_vulkan.h>

void MouseMovementEvent(SDL_Event& event, void* ref)
{
	phx::InputHandler* inputHander = reinterpret_cast<phx::InputHandler*>(ref);

	inputHander->MouseMoveEvent(event.motion.xrel, event.motion.yrel);
}

void MouseKeyEvent(SDL_Event& event, void* ref)
{
	phx::InputHandler* inputHander = reinterpret_cast<phx::InputHandler*>(ref);

	bool pressed = event.button.state == SDL_PRESSED;
	switch (event.button.button)
	{
	case SDL_BUTTON_LEFT:
		inputHander->MouseClickEvent(0, pressed);
		break;
	case SDL_BUTTON_RIGHT:
		inputHander->MouseClickEvent(1, pressed);
		break;
	case SDL_BUTTON_MIDDLE:
		inputHander->MouseClickEvent(2, pressed);
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
	mMouseMoveX = 0.0f;
	mMouseMoveY = 0.0f;
	window->AddEventCallback(SDL_MOUSEBUTTONDOWN, MouseKeyEvent, this);
	window->AddEventCallback(SDL_MOUSEBUTTONUP, MouseKeyEvent, this);

	window->AddEventCallback(SDL_KEYDOWN, WindowKeyEvent, this);
	window->AddEventCallback(SDL_KEYUP, WindowKeyEvent, this);

	window->AddEventCallback(SDL_MOUSEMOTION, MouseMovementEvent, this);

	mInvalidLastMouse = true;

	for (int i = 0; i < 256; ++i)
	{
		mKeys[i] = false;
	}

	for (int i = 0 ; i < 3; i++)
	{
		mMouseKeys[i] = false;
	}
}

phx::InputHandler::~InputHandler()
{

}

void phx::InputHandler::Update()
{
	mMouseMoveX = 0.0f;
	mMouseMoveY = 0.0f;
}

void phx::InputHandler::KeyEvent(int key, bool pressed)
{
	if (key >= 256)
		return;
	mKeys[key] = pressed;
}

void phx::InputHandler::MouseClickEvent(int mouseKey, bool pressed)
{
	if (mouseKey >= 3)
		return;
	mMouseKeys[mouseKey] = pressed;
}

void phx::InputHandler::MouseMoveEvent(int x, int y)
{
	if (!mInvalidLastMouse)
	{
		// Temp scale
		float mouseScale = 0.06f;
		mMouseMoveX      = (float) x * mouseScale;
		mMouseMoveY      = (float) y * mouseScale;
	}
	mInvalidLastMouse = false;
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

float phx::InputHandler::GetMouseMoveX() { return mMouseMoveX; }

float phx::InputHandler::GetMouseMoveY() { return mMouseMoveY; }

