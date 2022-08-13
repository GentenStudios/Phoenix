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

		void Update();

		void KeyEvent(int key, bool pressed);

		void MouseClickEvent(int mouseKey, bool pressed);

		void MouseMoveEvent(int x, int y);

		bool IsPressed(int key);

		bool IsMousePressed(int key);

		float GetMouseMoveX();

		float GetMouseMoveY();

	private:

		Window* mWindow;

		float mMouseMoveX;
		float mMouseMoveY;

		bool mInvalidLastMouse;

		bool mKeys[256];

		bool mMouseKeys[3];
	};



} // namespace phx
