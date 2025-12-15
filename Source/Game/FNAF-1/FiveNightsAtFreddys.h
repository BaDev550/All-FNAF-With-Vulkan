#pragma once
#include "Game/Game.h"

#include "Graphics/Buffers/Framebuffer.h"
#include "Objects/CameraObject.h"

#include "Animatronic_Freddy.h"
#include "Animatronic_Bonnie.h"
#include "Animatronic_Chica.h"

class FiveNightsAtFreddys : public Game {
public:
	enum Night : uint8_t { NIGHT1, NIGHT2, NIGHT3, NIGHT4, NIGHT5 } _Night = NIGHT1;

	FiveNightsAtFreddys(Night night = NIGHT1);
	~FiveNightsAtFreddys();
	virtual void Update(float dt) override;

	enum CameraID : uint8_t {
		CAM1A,
		CAM2B,
		CAM2A,
		CAM3,
		CAM4B,
		CAM4A,
		CAM5,
		CAM6,
		CAM7
	} _ActiveCam = CAM1A;
private:
	void DrawObjectDebugPanel();
	void DrawLightsDebugPanel();
	void DrawSecurityCameraFeedPanel();
	void DrawFnafSideHUD();
	void DrawBottomHUD();
private:
	struct PhoneGuy {
		std::string ng1 = "Resources/Audio/voiceover1c.wav";
		std::string ng2 = "Resources/Audio/voiceover2a.wav";
		std::string ng3 = "Resources/Audio/voiceover3.wav";
		std::string ng4 = "Resources/Audio/voiceover4.wav";
		std::string ng5 = "Resources/Audio/voiceover5.wav";
		bool InCall = false;
	} _Phoneguy;

	std::map<CameraID, CameraObject> _Cameras = {
		{CAM1A, CameraObject("SHOW_STAGE_CAM", {-1.4f, -1.1f, 8.8f}, {-0.4f, 0.9f, 0.0f})},
		{CAM2A, CameraObject("LEFT_HALLWAY_CAM",      { -1.0f, -1.3f, -0.3f}, {-0.5f, 0.0f, 0.0f})},
		{CAM2B, CameraObject("LEFT_HALLWAY_BACK_CAM", { -1.0f, -1.3f, -0.3f}, {-0.5f, -2.7f, 0.0f})},
		{CAM3,  CameraObject("SUPPLY_CLOSET_CAM",  {-3.0f, -1.3f, 1.8f}, {-0.7f, 0.5f, 0.0f})},
		{CAM4A, CameraObject("RIGHT_HALLWAY_CAM",      {1.0f, -1.3f, -0.3f}, {-0.5, 0.0f, 0.0f})},
		{CAM4B, CameraObject("RIGHT_HALLWAY_BACK_CAM", {1.0f, -1.3f, -0.3f}, {-0.5, 2.7f, 0.0f})},
		{CAM5,  CameraObject("BACKSTAGE_CAM",  {-5.5f, -1.2f, 5.1f}, {0.0f, 0.0f, 0.0f}) },
		{CAM6,  CameraObject("KITCHEN_CAM",    {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f})},
		{CAM7,  CameraObject("RESTROOMS_CAM",  {5.5f, -0.8f, 7.8f}, {0.0f, 3.1f, 0.0f})}
	};
	Framebuffer _CameraFramebuffer{ 530, 530 };
	CameraObject* _ActiveCamera = &_Cameras[CAM1A];
	bool _TabletIsOpened = false;
	bool _CanSelectCamera = true;

	enum DoorState : uint8_t {
		L_CLOSED,
		R_CLOSED,
		BOTH_CLOSED,
		ALL_OPEN,
		BROKEN
	} _DoorState = ALL_OPEN;

	CameraObject _FreeCam;
	CameraObject _GameCam{"GAME_CAM", {0.0f, -0.75f, -1.4f}, {0.0f, 0.0f, 0.0f}};

	std::vector<PointLight> _PointLights = {
		PointLight({0.0f, -1.2f, 0.0f},  0.2f),
		PointLight({0.0f, -1.2f, 7.2f},  4.6f),
		PointLight({-5.9f, -1.1f, 7.4f}, 1.0f),
		PointLight({-2.6f, -1.5f, 1.9f}, 1.0f),

		PointLight({1.0f, -1.2f, 0.0f},  0.0f),
		PointLight({-1.0f, -1.2f, 0.0f}, 0.0f)
	};
	PointLight& _RDoorLight = _PointLights[4];
	PointLight& _LDoorLight = _PointLights[5];
	DirectionalLight _SunLight;

	Keyboard_Movement _KeyboardMovement;

	GameObject* _SelectedObject = nullptr;
	MEM::Scope<SceneRenderer> _SceneRenderer;
	MEM::Ref<Pipeline> _MainShader = nullptr;
	MEM::Ref<Pipeline> _CameraShader = nullptr;

	SoundBuffer _GeneralSoundEffectsBuffer;

	SoundSource _DoorCloseSoundEffect;
	SoundSource _BrokenDoorCloseSoundEffect;
	SoundSource _OpenTabletSoundEffect;
	SoundSource _TabletPutDownSoundEffect;
	SoundSource _TabletBlipSoundEffect;
	SoundSource _AmbientSoundEffect;
	SoundSource _FanSoundEffect;
	SoundSource _PhoneguySoundEffect;
	SoundSource _DoorLightSoundEffect;
	SoundSource _PowerDownSoundEffect;
	SoundSource _BellsSoundEffect;
	SoundSource _AnimatronicAtDoorSoundEffect;
	SoundSource _TabletBrokeSoundEffect;

	std::array<Mesh*, 2> _DoorMeshes;

	std::string DoorStateToString(DoorState state) {
		switch (state)
		{
		case FiveNightsAtFreddys::L_CLOSED: return "Left Closed";
		case FiveNightsAtFreddys::R_CLOSED: return "Right Closed";
		case FiveNightsAtFreddys::BOTH_CLOSED: return "Both Closed";
		case FiveNightsAtFreddys::ALL_OPEN: return "All Open";
		case FiveNightsAtFreddys::BROKEN: return "Broken";
		}
	}
	void CountTheClockTime(float dt);
	void GameOver(bool won = false);
	void HoldDoorLight(PointLight& light, bool held, bool& wasHeld);
	int GetNightMultiplier() const;
	int CalculatePowerUsage() const;
	void CheckPowerLeft();
	void CheckDoorState();
	void OpenLeftDoor();
	void OpenRightDoor();
	void ToggleTablet();
	void BrokeTablet();
	bool FlashActive(DoorSide side);

	void CheckAnimatronic(Animatronic& anim, float dt);
private:
	float _PowerLeft = 100.0f;
	float _NightProgress = 0.0f;
	bool _HasPower = true;
	bool _GameOver = false;
	uint8_t _PowerUsage = 1;

	int _CurrentHour = 12;
	float _TimeElapsed = 0.0f;
	const float TIME_PER_HOUR = 30.0f;

	const bool _DEBUG_GAME = false;

	bool _HasAnimatronicAtRightDoor = false;
	bool _HasAnimatronicAtLeftDoor = false;
	bool _GettingJumpscared = false;
	Animatronic* _Jumpscarer = nullptr;
	float _JumpscareTimer = 0.0f;
private:
	MEM::Ref<Animatronic_Freddy> _Freddy;
	MEM::Ref<Animatronic_Bonnie> _Bonnie;
	MEM::Ref<Animatronic_Chica> _Chica;
};