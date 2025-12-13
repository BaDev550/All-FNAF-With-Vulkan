#pragma once
#include "Graphics/Render/SceneRenderer.h"
#include "Graphics/Camera.h"
#include "KeyboardMovement.h"
#include "Objects/GameObject.h"
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

class FiveNightsAtFreddys : public Game {
public:
	FiveNightsAtFreddys();
	~FiveNightsAtFreddys();
	virtual void Update(float dt) override;
private:
	Camera _GameCamera;
	Camera _FreeCam;

	PointLight _TestLight;
	DirectionalLight _TestSunLight;
	GameObject _CameraObject;
	GameObject* _SelectedObject = nullptr;
	Keyboard_Movement _KeyboardMovement;
	MEM::Scope<SceneRenderer> _SceneRenderer;
	MEM::Ref<Pipeline> _MainShader = nullptr;

};