#include "gpch.h"
#include "Window.h"

static bool s_GLFWInitilized = false;
Window::Window(const WindowSpecifications windowSpecs)
	: _Specs(windowSpecs)
{
	if (!s_GLFWInitilized) {
		int success = glfwInit();
		ASSERT(success, "Failed to create glfw instance");
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	}
	_Handle = glfwCreateWindow(_Specs.width, _Specs.height, _Specs.windowName.c_str(), NULL, NULL);
	ASSERT(_Handle, "Failed to create glfw window");
	glfwMakeContextCurrent(_Handle);
	glfwSetWindowUserPointer(_Handle, this);
}

Window::~Window() {
	glfwDestroyWindow(_Handle);
	glfwTerminate();
}

glm::vec2 Window::GetExtend() const
{
	return { _Specs.width, _Specs.height };
}

void Window::ResetWindowresizedFlag()
{
	_WindowResized = false;
}

void Window::PollEvents() {
	glfwPollEvents();
}

const bool Window::ShoudClose() const {
	return glfwWindowShouldClose(_Handle);
}

void Window::framebufferResizeCallback(GLFWwindow* window, int width, int height)
{
	auto cwindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
	cwindow->_WindowResized = true;
	cwindow->_Specs.width = width;
	cwindow->_Specs.height = height;
}
