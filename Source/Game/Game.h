#pragma once
#include "Graphics/Render/SceneRenderer.h"
#include "Graphics/Camera.h"
#include "KeyboardMovement.h"
#include "Objects/GameObject.h"
#include "Audio/SoundDevice.h"
#include "Audio/SoundBuffer.h"
#include "Audio/SoundSource.h"
#include "Engine/ImGuiLayer.h"

class Game {
public:
	Game();
	virtual void Update(float dt) = 0;
protected:
	Renderer& _Renderer;
	MEM::Scope<ImGuiLayer> _ImGuiLayer;
	std::vector<GameObject> _GameObjects;
};