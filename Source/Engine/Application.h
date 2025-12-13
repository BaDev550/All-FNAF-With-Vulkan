#pragma once
#include "Window/Window.h"
#include "Graphics/Device.h"
#include "Graphics/Render/Renderer.h"
#include "Game/Game.h"

#include <iostream>
#include <string>

struct ApplicationSpecifications {
	std::string name;
	int width;
	int height;
	uint32_t appVersion;
	ApplicationSpecifications() : name("Application"), width(800), height(800), appVersion(1) {}
	ApplicationSpecifications(const std::string& appName, int appWidth, int appHeight, uint32_t AppVersion) 
		: name(appName), width(appWidth), height(appHeight), appVersion(AppVersion) { }
};

class Application {
public:
	Application(const ApplicationSpecifications specs);
	~Application();
	static Application* Get() { return _Instance; }
	void Run();

	void SetGame(MEM::Ref<Game> game) { _Game = game; }
	Device& GetDevice() { return *_Device; }
	Window& GetWindow() { return *_Window; }
	Renderer& GetRenderer() { return *_Renderer; }
private:
	static Application* _Instance;
	ApplicationSpecifications _Specs;

	MEM::Ref<Game> _Game = nullptr;

	MEM::Scope<Window> _Window = nullptr;
	MEM::Scope<Device> _Device = nullptr;
	MEM::Scope<Renderer> _Renderer = nullptr;
};