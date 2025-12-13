#pragma once
#include <glm/vec2.hpp>
#include <GLFW/glfw3.h>

struct WindowSpecifications {
	std::string windowName;
	int width, height;

	WindowSpecifications(const std::string& Name = "Default", int Width = 800, int Height = 800) : width(Width), height(Height), windowName(Name) {}
};

class Window {
public:
	Window(const WindowSpecifications windowSpecs);
	~Window();

	glm::vec2 GetExtend() const;
	GLFWwindow* GetHandle() const { return _Handle; }
	void ResetWindowresizedFlag();
	void PollEvents();
	const bool WasWindowResized() const { return _WindowResized; }
	const bool ShoudClose() const;
	const int GetWidth() const { return _Specs.width; }
	const int GetHeight() const { return _Specs.height; }
private:
	static void framebufferResizeCallback(GLFWwindow* window, int width, int height);
	bool _WindowResized = false;

	GLFWwindow* _Handle;
	WindowSpecifications _Specs;
};