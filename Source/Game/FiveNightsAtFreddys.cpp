#include "gpch.h"
#include "FiveNightsAtFreddys.h"
#include "Engine/Application.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include <AL/al.h>

struct MapButton { float x; float y; const char* label; };

FiveNightsAtFreddys::FiveNightsAtFreddys()
{
	_SceneRenderer = MEM::CreateScope<SceneRenderer>(_Renderer);

	_MainShader = MEM::CreateRef<Pipeline>(
		"Resources/Shaders/shader.vert", 
		"Resources/Shaders/shader.frag", 
		_SceneRenderer->GetSceneInfo().GetLayoutHandles(),
		Application::Get()->GetRenderer().GetSwapChainRenderPass()
	);

	_CameraShader = MEM::CreateRef<Pipeline>(
		"Resources/Shaders/shader.vert", 
		"Resources/Shaders/shader.frag", 
		_SceneRenderer->GetSceneInfo().GetLayoutHandles(),
		_CameraFramebuffer.GetRenderPass()
	);

	{
		GameObject map = GameObject::CreateGameObject("MAP_CONST");
		map.GetTransform().scale = glm::vec3(0.5f);
		Drawnable drawingInfo = _SceneRenderer->CreateDrawnable(map, "Resources/Models/Fnaf1_MAP/MAP.obj");
		int doorIndex = 0;
		for (auto& mesh : drawingInfo.model->GetMeshes()) {
			if (mesh.GetName() == "DOOR_L" || mesh.GetName() == "DOOR_R") {
				mesh.SetIsVisible(false);
				_DoorMeshes[doorIndex] = &mesh;
				doorIndex++;
			}
		}
		_GameObjects.push_back(map);
	}
	{
		GameObject object = GameObject::CreateGameObject("Freddy");
		object.GetTransform().position = glm::vec3(0.0f, -0.1f, 9.5f);
		object.GetTransform().scale = glm::vec3(0.12f);
		_SceneRenderer->CreateDrawnable(object, "Resources/Models/Freddy/Freddy.obj");
		_GameObjects.push_back(object);
	}
	{
		GameObject object = GameObject::CreateGameObject("Bonnie");
		object.GetTransform().position = glm::vec3(-1.0f, -0.1f, 10.0f);
		object.GetTransform().rotation = glm::vec3(180.0f, 0.0f, 0.0f);
		object.GetTransform().scale = glm::vec3(0.12f);
		_SceneRenderer->CreateDrawnable(object, "Resources/Models/Bonnie/Bonnie.obj");
		_GameObjects.push_back(object);
	}
	{
		GameObject object = GameObject::CreateGameObject("Chica");
		object.GetTransform().position = glm::vec3(1.0f, -0.1f, 10.0f);
		object.GetTransform().rotation = glm::vec3(180.0f, 0.0f, 0.0f);
		object.GetTransform().scale = glm::vec3(0.12f);
		_SceneRenderer->CreateDrawnable(object, "Resources/Models/Chica/Chica.obj");
		_GameObjects.push_back(object);
	}

	_DoorCloseSoundEffect.SetBuffer(_GeneralSoundEffectsBuffer.AddSoundEffect("Resources/Audio/SFXBible_12478.wav"));
	_BrokenDoorCloseSoundEffect.SetBuffer(_GeneralSoundEffectsBuffer.AddSoundEffect("Resources/Audio/error.wav"));
	_OpenTabletSoundEffect.SetBuffer(_GeneralSoundEffectsBuffer.AddSoundEffect("Resources/Audio/CAMERA_VIDEO_LOA_60105303.wav"));
	_TabletPutDownSoundEffect.SetBuffer(_GeneralSoundEffectsBuffer.AddSoundEffect("Resources/Audio/put down.wav"));
	_TabletBlipSoundEffect.SetBuffer(_GeneralSoundEffectsBuffer.AddSoundEffect("Resources/Audio/blip3.wav"));

	std::string phoneGuySoundeffectPath;
	switch (_Night)
	{
	case FiveNightsAtFreddys::NIGHT1:
		phoneGuySoundeffectPath = _Phoneguy.ng1;
		break;
	case FiveNightsAtFreddys::NIGHT2:
		phoneGuySoundeffectPath = _Phoneguy.ng2;
		break;
	case FiveNightsAtFreddys::NIGHT3:
		phoneGuySoundeffectPath = _Phoneguy.ng3;
		break;
	case FiveNightsAtFreddys::NIGHT4:
		phoneGuySoundeffectPath = _Phoneguy.ng4;
		break;
	case FiveNightsAtFreddys::NIGHT5:
		phoneGuySoundeffectPath = _Phoneguy.ng5;
		break;
	}
	_PhoneguySoundEffect.SetBuffer(_GeneralSoundEffectsBuffer.AddSoundEffect(phoneGuySoundeffectPath.c_str()));
	_PhoneguySoundEffect.Play();
	_Phoneguy.InCall = true;

	_AmbientSoundEffect.SetBuffer(_GeneralSoundEffectsBuffer.AddSoundEffect("Resources/Audio/ambience2.wav"));
	_AmbientSoundEffect.SetLoopSound(true);
	_AmbientSoundEffect.Play();

	_FanSoundEffect.SetBuffer(_GeneralSoundEffectsBuffer.AddSoundEffect("Resources/Audio/Buzz_Fan_Florescent2.wav"));
	_FanSoundEffect.SetLoopSound(true);
	_FanSoundEffect.Play();
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

	_KeyboardMovement.MoveInPlaneXZ(Application::Get()->GetWindow().GetHandle(), dt, _FreeCam);
	_FreeCam.Update();

	_Renderer.Submit([&](VkCommandBuffer cmd) {
		if (_ActiveCamera) {
			_CameraFramebuffer.BeginRenderPass(cmd);
			_ActiveCamera->Update();
			_SceneRenderer->Draw(cmd, *_CameraShader, _GameObjects, _ActiveCamera->GetCamera(), 1);
			_CameraFramebuffer.EndRenderPass(cmd);
		}

		Commands::BeginRenderPass(cmd);
		Commands::SetViewport(cmd, _Renderer.GetSwapChainExtent().width, _Renderer.GetSwapChainExtent().height);
		_SceneRenderer->Draw(cmd, *_MainShader, _GameObjects, _FreeCam.GetCamera(), 0);

		_ImGuiLayer->BeginFrame();

		DrawObjectDebugPanel();
		DrawLightsDebugPanel();
		DrawGameDebugPanel();
		DrawSecurityCameraFeedPanel();

		_ImGuiLayer->EndFrame(cmd);
		Commands::EndRenderPass(cmd);
		});

	if (_DoorState != BROKEN) {
		if (_DoorMeshes[0]->IsVisible() && _DoorMeshes[1]->IsVisible())
			_DoorState = BOTH_CLOSED;
		else if (_DoorMeshes[0]->IsVisible())
			_DoorState = L_CLOSED;
		else if (_DoorMeshes[1]->IsVisible())
			_DoorState = R_CLOSED;
		else
			_DoorState = ALL_OPEN;
	}
}

void FiveNightsAtFreddys::DrawObjectDebugPanel()
{
	ImGui::Begin("Objects debug panel");
	for (auto& object : _GameObjects) {
		if (ImGui::Selectable(object.GetName().c_str()))
			_SelectedObject = &object;
	}
	if (_SelectedObject) {
		auto& transform = _SelectedObject->GetTransform();
		ImGui::DragFloat3("Position", glm::value_ptr(transform.position), 0.1f);
		ImGui::DragFloat3("Rotation", glm::value_ptr(transform.rotation), 0.1f);
		ImGui::DragFloat3("Scale", glm::value_ptr(transform.scale), 0.1f);
	}
	ImGui::End();
}

void FiveNightsAtFreddys::DrawLightsDebugPanel()
{
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
}

void FiveNightsAtFreddys::DrawGameDebugPanel()
{
	ImGui::Begin("Game");
	if (ImGui::Button("Break / Fix the doors")) { _DoorState = _DoorState == BROKEN ? ALL_OPEN : BROKEN; }

	if (ImGui::Button("Close / Open Left Door")) {
		if (_DoorState == BROKEN) {
			_BrokenDoorCloseSoundEffect.Play();
		}
		else {
			_DoorMeshes[0]->SetIsVisible(!_DoorMeshes[0]->IsVisible());
			_DoorCloseSoundEffect.Play();
		}
	}
	if (ImGui::Button("Close / Open Right Door")) {
		if (_DoorState == BROKEN) {
			_BrokenDoorCloseSoundEffect.Play();
		}
		else {
			_DoorMeshes[1]->SetIsVisible(!_DoorMeshes[1]->IsVisible());
			_DoorCloseSoundEffect.Play();
		}
	}

	if (_Phoneguy.InCall) {
		if (ImGui::Button("Close Call")) {
			_PhoneguySoundEffect.Stop();
			_Phoneguy.InCall = false;
		}
	}

	if (ImGui::Button("Open / Close Tablet")) {
		if (_TabletIsOpened) {
			_TabletPutDownSoundEffect.Play();
		}
		else {
			_OpenTabletSoundEffect.Play();
		}
		_TabletIsOpened = !_TabletIsOpened;
	}
	ImGui::Text(DoorStateToString(_DoorState).c_str());
	ImGui::End();
}

void FiveNightsAtFreddys::DrawSecurityCameraFeedPanel()
{
	if (_TabletIsOpened) {
		ImGui::Begin("Security Camera Feed");

		ImVec2 imageSize(530, 530);
		ImVec2 startPos = ImGui::GetCursorScreenPos();

		ImGui::Dummy(imageSize);
		ImGui::SetCursorScreenPos(startPos);
		
		if (_ActiveCamera) { ImGui::Image((ImTextureID)_CameraFramebuffer.GetDescriptorSet(), imageSize); }
		else {
			ImVec2 txtSz = ImGui::CalcTextSize("NO SIGNAL");
			ImGui::SetCursorScreenPos(ImVec2(startPos.x + (imageSize.x / 2 - txtSz.x / 2), startPos.y + (imageSize.y / 2 - txtSz.y / 2)));
			ImGui::Text("NO SIGNAL");
		}

		ImGuiStyle& style = ImGui::GetStyle();
		ImVec2 btnSize(40, 20);
		const float padding = 0.05f;
		static const std::map<FiveNightsAtFreddys::CameraID, MapButton> anchor_layout = {
			{FiveNightsAtFreddys::CAM4B, {1.00f - padding, 1.00f - padding, "4B"}},
			{FiveNightsAtFreddys::CAM4A, {1.00f - padding, 1.00f - padding - 0.07f, "4A"}},
			{FiveNightsAtFreddys::CAM6,  {1.00f - padding - 0.07f, 1.00f - padding, "6"}},
			{FiveNightsAtFreddys::CAM7,  {1.00f - padding - 0.07f, 1.00f - padding - 0.07f, "7"}},
			{FiveNightsAtFreddys::CAM3,  {1.00f - padding - 0.14f, 1.00f - padding, "3"}},
			{FiveNightsAtFreddys::CAM1A, {1.00f - padding - 0.14f, 1.00f - padding - 0.07f, "1A"}},
			{FiveNightsAtFreddys::CAM2B, {1.00f - padding - 0.21f, 1.00f - padding, "2B"}},
			{FiveNightsAtFreddys::CAM2A, {1.00f - padding - 0.21f, 1.00f - padding - 0.07f, "2A"}},
			{FiveNightsAtFreddys::CAM5,  {1.00f - padding - 0.28f, 1.00f - padding, "5"}},
		};

		for (auto& [camID, layout] : anchor_layout) {
			ImVec2 btnPos;
			btnPos.x = startPos.x + (imageSize.x * layout.x) - btnSize.x;
			btnPos.y = startPos.y + (imageSize.y * layout.y) - btnSize.y;

			ImGui::SetCursorScreenPos(btnPos);

			bool isActive = (_ActiveCam == camID);
			if (isActive) {
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.1f, 0.8f, 0.1f, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.9f, 0.2f, 1.0f));
			}
			else {
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 0.8f));
			}

			if (ImGui::Button(layout.label, btnSize)) {
				_ActiveCam = camID;
				if (_Cameras.find(camID) != _Cameras.end()) {
					_ActiveCamera = _ActiveCam == CAM6 ? nullptr : &_Cameras[camID];
					_TabletBlipSoundEffect.Play();
				}
			}

			if (isActive) { ImGui::PopStyleColor(2); }
			else { ImGui::PopStyleColor(1); }
		}
		ImGui::NewLine();
		ImGui::End();
	}
}
