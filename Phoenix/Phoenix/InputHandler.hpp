#pragma once

#include <memory>

#include <Globals/Globals.hpp>

class Window;

namespace phx
{

	class InputHandler
	{
	public:
		InputHandler(Window* window);

		~InputHandler();

		void KeyEvent(int key, bool pressed);

		void MouseEvent(int mouseKey, bool pressed);

		bool IsPressed(int key);

		bool IsMousePressed(int key);

	private:

		Window* mWindow;

		bool mKeys[256];

		bool mMouseKeys[3];
	};



} // namespace phx
