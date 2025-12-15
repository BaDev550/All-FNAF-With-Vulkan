#include "gpch.h"
#include "FiveNightsAtFreddys.h"
#include "Engine/Application.h"

#include <imgui.h>
#include <glm/gtc/type_ptr.hpp>

#include <AL/al.h>

struct MapButton { float x; float y; const char* label; };

FiveNightsAtFreddys::FiveNightsAtFreddys(Night night)
	: _Night(night)
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
		MEM::Ref<GameObject> map = GameObject::CreateGameObject("MAP_CONST");
		map->GetTransform().scale = glm::vec3(0.5f);
		Drawnable drawingInfo = _SceneRenderer->CreateDrawnable(*map, "Resources/Models/Fnaf1_MAP/MAP.obj");
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

	_Freddy = MEM::CreateRef<Animatronic_Freddy>(*_SceneRenderer);
	_Freddy->SetNightDifficulty(((int)_Night + 1));
	_Bonnie = MEM::CreateRef<Animatronic_Bonnie>(*_SceneRenderer);
	_Bonnie->SetNightDifficulty(((int)_Night + 1));
	_Chica = MEM::CreateRef<Animatronic_Chica>(*_SceneRenderer);
	_Chica->SetNightDifficulty(((int)_Night + 1));
	_GameObjects.push_back(_Freddy);
	_GameObjects.push_back(_Bonnie);
	_GameObjects.push_back(_Chica);

	_DoorCloseSoundEffect.SetBuffer(_GeneralSoundEffectsBuffer.AddSoundEffect("Resources/Audio/SFXBible_12478.wav"));
	_BrokenDoorCloseSoundEffect.SetBuffer(_GeneralSoundEffectsBuffer.AddSoundEffect("Resources/Audio/error.wav"));
	_OpenTabletSoundEffect.SetBuffer(_GeneralSoundEffectsBuffer.AddSoundEffect("Resources/Audio/CAMERA_VIDEO_LOA_60105303.wav"));
	_TabletPutDownSoundEffect.SetBuffer(_GeneralSoundEffectsBuffer.AddSoundEffect("Resources/Audio/put down.wav"));
	_TabletBlipSoundEffect.SetBuffer(_GeneralSoundEffectsBuffer.AddSoundEffect("Resources/Audio/blip3.wav"));
	_PowerDownSoundEffect.SetBuffer(_GeneralSoundEffectsBuffer.AddSoundEffect("Resources/Audio/powerdown.wav"));
	_BellsSoundEffect.SetBuffer(_GeneralSoundEffectsBuffer.AddSoundEffect("Resources/Audio/chimes 2.wav"));
	_AnimatronicAtDoorSoundEffect.SetBuffer(_GeneralSoundEffectsBuffer.AddSoundEffect("Resources/Audio/windowscare.wav"));
	_TabletBrokeSoundEffect.SetBuffer(_GeneralSoundEffectsBuffer.AddSoundEffect("Resources/Audio/COMPUTER_DIGITAL_L2076505.wav")); 

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

	_DoorLightSoundEffect.SetBuffer(_GeneralSoundEffectsBuffer.AddSoundEffect("Resources/Audio/BallastHumMedium2.wav"));
	_DoorLightSoundEffect.SetLoopSound(true);
}

FiveNightsAtFreddys::~FiveNightsAtFreddys()
{
	_MainShader = nullptr;
	_SceneRenderer = nullptr;
}

void FiveNightsAtFreddys::Update(float dt) {
	if (!_GameOver) {
		CountTheClockTime(dt);

		int usage = CalculatePowerUsage();
		int hourIndex = (_CurrentHour == 12) ? 0 : _CurrentHour;
		_PowerUsage = usage;
		_PowerLeft -= usage * (dt / 3.5f);
		_PowerLeft = glm::clamp(_PowerLeft, 0.0f, 100.0f);
		_HasPower = _PowerLeft != 0;
		_NightProgress = hourIndex / 6.0f;
		CheckDoorState();
		CheckPowerLeft();

		if (!_DEBUG_GAME) {
			_Freddy->SetNightProgress(_NightProgress);
			_Bonnie->SetNightProgress(_NightProgress);
			_Chica->SetNightProgress(_NightProgress);

			_Freddy->Update(dt);
			_Bonnie->Update(dt);
			_Chica->Update(dt);
			CheckAnimatronic(*_Freddy, dt);
			CheckAnimatronic(*_Bonnie, dt);
			CheckAnimatronic(*_Chica, dt);

			if (_TabletIsOpened &&
				(_Freddy->AnimatronicMovedThisFrame() ||
				_Bonnie->AnimatronicMovedThisFrame() ||
				_Chica->AnimatronicMovedThisFrame())) {
				BrokeTablet();
				_Freddy->ResetMovedState();
				_Bonnie->ResetMovedState();
				_Chica->ResetMovedState();
			}
			else {
				_CanSelectCamera = true;
			}
		}

		if (_GettingJumpscared) {
			_JumpscareTimer += dt;
			float randomTime = rand() % 2;
			if (_JumpscareTimer >= randomTime) {
				if (_Jumpscarer) {
					_Jumpscarer->Jumpscare();
					_GettingJumpscared = false;
					GameOver();
				}
			}
		}
	}

	_SceneRenderer->GetSceneInfo().lightEnviroment.PointLights.resize(1);
	_SceneRenderer->GetSceneInfo().lightEnviroment.PointLights = _PointLights;
	_SceneRenderer->GetSceneInfo().lightEnviroment.DirectionalLight[0] = _SunLight;
	_KeyboardMovement.MoveInPlaneXZ(Application::Get()->GetWindow().GetHandle(), dt, _FreeCam);
	_FreeCam.Update();
	_GameCam.Update();
	_Renderer.Submit([&](VkCommandBuffer cmd) {
		if (_ActiveCamera) {
			_CameraFramebuffer.BeginRenderPass(cmd);
			_ActiveCamera->Update();
			_SceneRenderer->Draw(cmd, *_CameraShader, _GameObjects, *_ActiveCamera, 1);
			_CameraFramebuffer.EndRenderPass(cmd);
		}

		Commands::BeginRenderPass(cmd);
		Commands::SetViewport(cmd, _Renderer.GetSwapChainExtent().width, _Renderer.GetSwapChainExtent().height);
		_SceneRenderer->Draw(cmd, *_MainShader, _GameObjects, _DEBUG_GAME ? _FreeCam : _GameCam, 0);

		_ImGuiLayer->BeginFrame();
		
		DrawFnafSideHUD();
		DrawBottomHUD();
		DrawSecurityCameraFeedPanel();

		_ImGuiLayer->EndFrame(cmd);
		Commands::EndRenderPass(cmd);
		});
}

void FiveNightsAtFreddys::DrawObjectDebugPanel()
{
	ImGui::Begin("Objects debug panel");
	for (auto& object : _GameObjects) {
		if (ImGui::Selectable(object->GetName().c_str()))
			_SelectedObject = object.get();
	}
	if (_SelectedObject) {
		auto& transform = _SelectedObject->GetTransform();
		ImGui::DragFloat3("Position", glm::value_ptr(transform.position), 0.1f);
		ImGui::DragFloat3("Rotation", glm::value_ptr(transform.rotation), 0.1f);
		ImGui::DragFloat3("Scale", glm::value_ptr(transform.scale), 0.1f);
	}

	if (ImGui::CollapsingHeader("Animatronics")) {
		if (ImGui::CollapsingHeader("Animatronic Freddy")) {

			if (ImGui::Button("Freddy move RIGHT_DOOR")) {
				_Freddy->MoveTo("RIGHT_DOOR");
			}
			if (ImGui::Button("Freddy move LEFT_DOOR")) {
				_Freddy->MoveTo("LEFT_DOOR");
			}
			if (ImGui::Button("Freddy move MAIN_STAGE")) {
				_Freddy->MoveTo("MAIN_STAGE");
			}
		}
	}
	ImGui::End();
}

void FiveNightsAtFreddys::DrawLightsDebugPanel()
{
	ImGui::Begin("Light Debug Panel");
	if (ImGui::CollapsingHeader("Point Light")) {
		uint32_t index = 0;
		for (auto& light : _PointLights) {
			std::string name = "Light - " + std::to_string(index);
			if (ImGui::CollapsingHeader(name.c_str())) {
				ImGui::DragFloat3("Position", glm::value_ptr(light.Position), 0.1f);
				ImGui::DragFloat("Intensity", &light.Intensity, 0.1f);
				ImGui::ColorPicker3("Color", glm::value_ptr(light.Color), ImGuiColorEditFlags_NoPicker);
				if (ImGui::Button("Copy vec3 coords")) {
					TransformComponent::CopyVec3PositionCoords(light.Position);
				}
			}
			index++;
		}
	}
	if (ImGui::CollapsingHeader("Sun Light")) {
		ImGui::DragFloat3("Rotation", glm::value_ptr(_SunLight.Direction), 0.1f);
		ImGui::DragFloat("Intensity", &_SunLight.Intensity, 0.1f);
		ImGui::ColorPicker3("Color", glm::value_ptr(_SunLight.Color), 0.1f);
	}
	ImGui::End();
}

void FiveNightsAtFreddys::CountTheClockTime(float dt)
{
	if (_HasPower && !_GettingJumpscared) {
		_TimeElapsed += dt;
		if (_TimeElapsed >= TIME_PER_HOUR)
		{
			_CurrentHour++;
			_TimeElapsed = 0.0f;

			if (_CurrentHour == 13)
				_CurrentHour = 1;
		}

		if (_CurrentHour == 6)
		{
			GameOver(true);
		}
	}
}

void FiveNightsAtFreddys::GameOver(bool won)
{
	if (_GameOver)
		return;

	_AmbientSoundEffect.Stop();
	_FanSoundEffect.Stop();
	_PhoneguySoundEffect.Stop();

	_GameOver = true;

	if (won) {
		_BellsSoundEffect.Play();
	}
}

void FiveNightsAtFreddys::HoldDoorLight(PointLight& light, bool held, bool& wasHeld)
{
	if (!_PowerLeft)
		return;

	if (held && !wasHeld){
		_DoorLightSoundEffect.Play();
	}

	if (!held && wasHeld) {
		_DoorLightSoundEffect.Stop();
	}

	light.Intensity = held ? 1.0f : 0.0f;
	wasHeld = held;
}

int FiveNightsAtFreddys::GetNightMultiplier() const
{
	switch (_Night)
	{
	case FiveNightsAtFreddys::NIGHT1: return 1;
	case FiveNightsAtFreddys::NIGHT2: return 2;
	case FiveNightsAtFreddys::NIGHT3: return 3;
	case FiveNightsAtFreddys::NIGHT4: return 4;
	case FiveNightsAtFreddys::NIGHT5: return 5;
	}
}

int FiveNightsAtFreddys::CalculatePowerUsage() const
{
	int usage = 1;
	if (_DoorMeshes[0]->IsVisible()) usage++;
	if (_DoorMeshes[1]->IsVisible()) usage++;

	if (_LDoorLight.Intensity > 0.0f) usage++;
	if (_RDoorLight.Intensity > 0.0f) usage++;

	if (_TabletIsOpened) usage++;
	return usage * GetNightMultiplier();
}

void FiveNightsAtFreddys::CheckPowerLeft()
{
	if (!_PowerLeft && !_GettingJumpscared && !_DEBUG_GAME) {
		_GettingJumpscared = true;
		OpenLeftDoor();
		OpenRightDoor();
		ToggleTablet();
		_DoorState = BROKEN;
		_PowerDownSoundEffect.Play();

		_GettingJumpscared = true;
		_Jumpscarer = _Freddy.get();
	}
}

void FiveNightsAtFreddys::CheckDoorState()
{
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

void FiveNightsAtFreddys::OpenLeftDoor()
{
	if (_DoorState == BROKEN) {
		_BrokenDoorCloseSoundEffect.Play();
	}
	else {
		_DoorMeshes[0]->SetIsVisible(!_DoorMeshes[0]->IsVisible());
		_DoorCloseSoundEffect.Play();
	}
}

void FiveNightsAtFreddys::OpenRightDoor()
{
	if (_DoorState == BROKEN) {
		_BrokenDoorCloseSoundEffect.Play();
	}
	else {
		_DoorMeshes[1]->SetIsVisible(!_DoorMeshes[1]->IsVisible());
		_DoorCloseSoundEffect.Play();
	}
}

void FiveNightsAtFreddys::ToggleTablet()
{
	if (_PowerLeft) {
		if (_TabletIsOpened) {
			_TabletPutDownSoundEffect.Play();
		}
		else {
			_OpenTabletSoundEffect.Play();
		}
		_TabletIsOpened = !_TabletIsOpened;
	}
	else {
		_TabletPutDownSoundEffect.Play();
		_TabletIsOpened = false;
	}
}

void FiveNightsAtFreddys::BrokeTablet()
{
	_TabletBrokeSoundEffect.Play();
	_ActiveCamera = nullptr;
	_CanSelectCamera = false;
}

bool FiveNightsAtFreddys::FlashActive(DoorSide side)
{
	if (side == DoorSide::LEFT && _LDoorLight.Intensity > 0.0f) return true;
	if (side == DoorSide::RIGHT && _RDoorLight.Intensity > 0.0f) return true;
	return false;
}

void FiveNightsAtFreddys::CheckAnimatronic(Animatronic& anim, float dt)
{
	if (anim.GetState() == AnimatronicState::AT_DOOR) {
		bool doorClosed =
			(anim.GetDoorSide() == DoorSide::LEFT && (_DoorState == L_CLOSED || _DoorState == BOTH_CLOSED)) ||
			(anim.GetDoorSide() == DoorSide::RIGHT && (_DoorState == R_CLOSED || _DoorState == BOTH_CLOSED));

		if (!doorClosed) {
			anim.AddAttackTime(dt);

			if ((anim.GetDoorSide() == DoorSide::RIGHT && FlashActive(DoorSide::RIGHT)) && !_HasAnimatronicAtRightDoor) {
				_HasAnimatronicAtRightDoor = true;
				_AnimatronicAtDoorSoundEffect.PlayOnce();
			}

			if ((anim.GetDoorSide() == DoorSide::LEFT && FlashActive(DoorSide::LEFT)) && !_HasAnimatronicAtLeftDoor) {
				_HasAnimatronicAtLeftDoor = true;
				_AnimatronicAtDoorSoundEffect.PlayOnce();
			}

			if (anim.GetAttackTime() >= anim.GetAttackDelay() && !_GettingJumpscared) {
				_GettingJumpscared = true;
				_DoorState = BROKEN;
				_Jumpscarer = &anim;
			}
		}
		else {
			anim.ResetAttackTimer();

			if (anim.GetTimeAtDoor() >= anim.GetMaxTimeAtDoor())
				anim.TryRetreat();
		}
	}
	else {
		_HasAnimatronicAtRightDoor = false;
		_HasAnimatronicAtLeftDoor = false;
	}
}

void FiveNightsAtFreddys::DrawSecurityCameraFeedPanel()
{
	if (_TabletIsOpened) {
		ImGui::Begin("Security Camera Feed", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);

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
				if (_CanSelectCamera) {
					_ActiveCam = camID;
					if (_Cameras.find(camID) != _Cameras.end()) {
						_ActiveCamera = _ActiveCam == CAM6 ? nullptr : &_Cameras[camID];
						_TabletBlipSoundEffect.Play();
					}
				}
			}

			if (isActive) { ImGui::PopStyleColor(2); }
			else { ImGui::PopStyleColor(1); }
		}
		ImGui::NewLine();
		ImGui::End();
	}
}

void FiveNightsAtFreddys::DrawBottomHUD()
{
	ImGuiIO& io = ImGui::GetIO();
	ImVec2 screen = io.DisplaySize;

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoDecoration |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNav;

	const float bottomOffset = 20.0f;
	const float panelWidth = 200.0f;
	const float panelHeight = 80.0f;
	const float tabletBtnHeight = 40.0f;

	ImVec2 statusPos = ImVec2(screen.x - panelWidth - bottomOffset, screen.y - panelHeight - bottomOffset);
	const char* am_pm = (_CurrentHour == 12 || _CurrentHour == 6) ? "AM" : "AM";
	ImGui::SetNextWindowPos(statusPos);
	ImGui::SetNextWindowSize(ImVec2(panelWidth, panelHeight));

	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.1f, 0.1f, 0.8f));
	ImGui::Begin("FNAF_StatusPanel", nullptr, flags);

	ImGui::Text("USAGE: %d", _PowerUsage);
	ImGui::Text("POWER LEFT: %.1f%%", _PowerLeft);

	ImGui::Separator();
	ImGui::Text("NIGHT %d | %d %s", (_Night + 1), _CurrentHour, am_pm);

	ImGui::End();
	ImGui::PopStyleColor(1);

	float tabletBtnWidth = 100.0f;
	ImVec2 tabletPos = ImVec2(screen.x * 0.5f - tabletBtnWidth * 0.5f, screen.y - tabletBtnHeight - 5.0f);
	ImGui::SetNextWindowPos(tabletPos);
	ImGui::SetNextWindowSize(ImVec2(tabletBtnWidth, tabletBtnHeight));

	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f); // Round corners for the tablet
	ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.15f, 0.15f, 0.15f, 0.9f));
	ImGui::Begin("FNAF_TabletToggle", nullptr, flags | ImGuiWindowFlags_NoTitleBar);

	const char* tabletLabel = _TabletIsOpened ? "PUT DOWN" : "CAMERAS";

	if (ImGui::Button(tabletLabel, ImVec2(tabletBtnWidth, tabletBtnHeight))) {
		ToggleTablet();
	}

	ImGui::End();
	ImGui::PopStyleColor(1);
	ImGui::PopStyleVar(1);

	if (_Phoneguy.InCall) {
		float callBtnWidth = 120.0f;
		float callBtnHeight = 40.0f;
		ImVec2 callPos = ImVec2(screen.x - callBtnWidth - bottomOffset, screen.y - panelHeight - bottomOffset - callBtnHeight - 10.0f);

		ImGui::SetNextWindowPos(callPos);
		ImGui::SetNextWindowSize(ImVec2(callBtnWidth, callBtnHeight));

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.8f, 0.1f, 0.1f, 1.0f));
		ImGui::Begin("FNAF_CallClose", nullptr, flags | ImGuiWindowFlags_NoTitleBar);

		if (ImGui::Button("CLOSE CALL", ImVec2(callBtnWidth, callBtnHeight))) {
			_PhoneguySoundEffect.Stop();
			_Phoneguy.InCall = false;
		}

		ImGui::End();
		ImGui::PopStyleColor(1);
	}
}

void FiveNightsAtFreddys::DrawFnafSideHUD()
{
	ImGuiIO& io = ImGui::GetIO();
	ImVec2 screen = io.DisplaySize;

	const ImVec2 btnSize(90, 60);
	const float sideOffset = 20.0f;
	const float verticalCenter = screen.y * 0.5f;

	static bool leftLightHeld = false;
	static bool rightLightHeld = false;

	ImGuiWindowFlags flags =
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoBackground |
		ImGuiWindowFlags_NoBringToFrontOnFocus |
		ImGuiWindowFlags_NoNav |
		ImGuiWindowFlags_AlwaysAutoResize;

	ImGui::SetNextWindowPos(ImVec2(
		sideOffset,
		verticalCenter - btnSize.y * 1.5f
	));

	ImGui::Begin("FNAF_LeftControls", nullptr, flags);

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.85f, 0.85f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	if (ImGui::Button("LDOOR", btnSize)) {
		OpenLeftDoor();
	}
	ImGui::PopStyleColor(2);

	ImGui::Dummy(ImVec2(1.0f, 10.0f));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.05f, 0.05f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.1f, 0.1f, 1.0f));

	if (ImGui::Button("LIGHT##Left", btnSize)) {}
	bool held = ImGui::IsItemActive();
	HoldDoorLight(_LDoorLight, held, leftLightHeld);

	ImGui::PopStyleColor(2);

	ImGui::End();

	ImGui::SetNextWindowPos(ImVec2(
		screen.x - sideOffset - btnSize.x,
		verticalCenter - btnSize.y * 1.5f
	));

	ImGui::Begin("FNAF_RightControls", nullptr, flags);
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.85f, 0.85f, 0.85f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
	if (ImGui::Button("RDOOR", btnSize)) {
		OpenRightDoor();
	}
	ImGui::PopStyleColor(2);

	ImGui::Dummy(ImVec2(1.0f, 10.0f));

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.6f, 0.05f, 0.05f, 1.0f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.9f, 0.1f, 0.1f, 1.0f));

	if (ImGui::Button("LIGHT##Right", btnSize)) {}
	held = ImGui::IsItemActive();
	HoldDoorLight(_RDoorLight, held, rightLightHeld);

	ImGui::PopStyleColor(2);

	ImGui::End();
}