#include "gpch.h"
#include "Application.h"
#include <chrono>
#include <numeric>

Application* Application::_Instance = nullptr;
Application::Application(const ApplicationSpecifications specs)
	: _Specs(specs)
{
	ASSERT(!_Instance, "Application already exists");
	LOG_INFO("Application created");
	_Instance = this;

	WindowSpecifications windowSpecs{};
	windowSpecs.windowName = specs.name;
	windowSpecs.width = specs.width;
	windowSpecs.height = specs.height;
	_Window = MEM::CreateScope<Window>(windowSpecs);
	_Device = MEM::CreateScope<Device>(*_Window, _Specs);
	_Renderer = MEM::CreateScope<Renderer>();
	_SoundDevice = MEM::CreateScope<SoundDevice>();
}

Application::~Application()
{
	_Game = nullptr;
	_Renderer = nullptr;
	_Device = nullptr;
	_Window = nullptr;
}

void Application::Run()
{
	auto currentTime = std::chrono::high_resolution_clock::now();
	while (!_Window->ShoudClose()) {
		auto newTime = std::chrono::high_resolution_clock::now();
		float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
		currentTime = newTime;

		_Game->Update(deltaTime);
		_Window->PollEvents();
	}
	vkDeviceWaitIdle(_Device->LogicalDevice());
}
