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

