#pragma once

#include <GLFW/glfw3.h>
namespace PVEngine
{
	class Window
	{
	public:
		Window();
		~Window();

		int Create();

		GLFWwindow* window;

		//dimensions for the window
		int windowWidth = 800;
		int windowHeight = 600;
	private:

		
	};
}

