#include "gpch.h"
#include "Game.h"
#include "Engine/Application.h"

Game::Game()
	: _Renderer(Application::Get()->GetRenderer())
{
	_ImGuiLayer = MEM::CreateScope<ImGuiLayer>();
}