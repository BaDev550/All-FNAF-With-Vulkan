#include "gpch.h"
#include "Game.h"
#include "Engine/Application.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

FiveNightsAtFreddys::FiveNightsAtFreddys()
{
	_SceneRenderer = MEM::CreateScope<SceneRenderer>(_Renderer);
	_MainShader = MEM::CreateRef<Pipeline>("Resources/Shaders/shader.vert", "Resources/Shaders/shader.frag", _SceneRenderer->GetSceneInfo().GetLayoutHandles());
	{
		GameObject map = GameObject::CreateGameObject("MAP_CONST");
		map.GetTransform().scale = glm::vec3(0.5f);
		_SceneRenderer->CreateDrawnable(map, "Resources/Models/Fnaf1_MAP/MAP.obj");
		_GameObjects.push_back(map);
	}
	{
		GameObject object = GameObject::CreateGameObject("Freddy");
		object.GetTransform().position = glm::vec3(0.0f, -0.1f, 9.5f);
		object.GetTransform().scale = glm::vec3(0.1f);
		_SceneRenderer->CreateDrawnable(object, "Resources/Models/Freddy/freddy.obj");
		_GameObjects.push_back(object);
	}
}

FiveNightsAtFreddys::~FiveNightsAtFreddys()
{
	_MainShader = nullptr;
	_SceneRenderer = nullptr;
}

void FiveNightsAtFreddys::Update(float dt) {
	_SceneRenderer->GetSceneInfo().lightEnviroment.PointLights.resize(1);
	_SceneRenderer->GetSceneInfo().lightEnviroment.PointLights[0] = _TestLight;
	_SceneRenderer->GetSceneInfo().lightEnviroment.DirectionalLight[0] = _TestSunLight;

	_KeyboardMovement.MoveInPlaneXZ(Application::Get()->GetWindow().GetHandle(), dt, _CameraObject);
	_GameCamera.SetPerspectiveProjection(45.0f, _Renderer.GetAspectRatio(), 0.1f, 100.0f);
	_GameCamera.SetViewYXZ(_CameraObject.GetTransform().position, _CameraObject.GetTransform().rotation);

	_ImGuiLayer->BeginFrame();
	
	ImGui::Begin("Objects debug panel");
	for (auto& object : _GameObjects) {
		if (ImGui::Selectable(object.GetName().c_str()))
			_SelectedObject = &object;
	}
	ImGui::End();

	if (_SelectedObject) {
		ImGui::Begin(_SelectedObject->GetName().c_str());
		auto& transform = _SelectedObject->GetTransform();
		ImGui::DragFloat3("Position", glm::value_ptr(transform.position), 0.1f);
		ImGui::DragFloat3("Rotation", glm::value_ptr(transform.rotation), 0.1f);
		ImGui::DragFloat3("Scale", glm::value_ptr(transform.scale), 0.1f);

		for (auto& mesh : _SceneRenderer->GetDrawnable(_SelectedObject->GetID()).model->GetMeshes()) {
			if (ImGui::Selectable(mesh.GetName().c_str())) {
				mesh.SetIsVisible(!mesh.IsVisible());
			}
		}

		ImGui::End();
	}

	ImGui::Begin("Light Debug Panel");
	if (ImGui::CollapsingHeader("Point Light")) {
		ImGui::DragFloat3("Position", glm::value_ptr(_TestLight.Position), 0.1f);
		ImGui::ColorPicker3("Color", glm::value_ptr(_TestLight.Color), 0.1f);
		ImGui::DragFloat("Intensity", &_TestLight.Intensity, 0.1f);
	}
	if (ImGui::CollapsingHeader("Sun Light")) {
		ImGui::DragFloat3("Rotation", glm::value_ptr(_TestSunLight.Direction), 0.1f);
		ImGui::ColorPicker3("Color", glm::value_ptr(_TestSunLight.Color), 0.1f);
		ImGui::DragFloat("Intensity", &_TestSunLight.Intensity, 0.1f);
	}
	ImGui::End();

	_SceneRenderer->Draw(*_MainShader, _GameObjects, _GameCamera, [&](VkCommandBuffer cmd) { _ImGuiLayer->EndFrame(cmd); });
}