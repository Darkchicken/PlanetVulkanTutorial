#include "Window.h"

namespace PVEngine
{

	Window::Window()
	{
	}
	

	Window::~Window()
	{
	}

	int Window::Create()
	{
		glfwInit();

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(windowWidth, windowHeight, "Planet Vulkan", nullptr, nullptr);

		return 0;
	}
}
