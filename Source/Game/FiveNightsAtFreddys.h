#pragma once
#include "Game.h"

#include "Graphics/Buffers/Framebuffer.h"
#include "Objects/CameraObject.h"

class FiveNightsAtFreddys : public Game {
public:
	FiveNightsAtFreddys();
	~FiveNightsAtFreddys();
	virtual void Update(float dt) override;

	enum CameraID {
		CAM1A,
		CAM2B,
		CAM2A,
		CAM3,
		CAM4B,
		CAM4A,
		CAM5,
		CAM6,
		CAM7
	} _ActiveCam = CAM1A;;
private:
	void DrawObjectDebugPanel();
	void DrawLightsDebugPanel();
	void DrawGameDebugPanel();
	void DrawSecurityCameraFeedPanel();
private:
	enum Night { NIGHT1, NIGHT2, NIGHT3, NIGHT4, NIGHT5 } _Night = NIGHT1;
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

	enum DoorState {
		L_CLOSED,
		R_CLOSED,
		BOTH_CLOSED,
		ALL_OPEN,
		BROKEN
	} _DoorState = ALL_OPEN;

	CameraObject _FreeCam;

	PointLight _TestLight;
	DirectionalLight _TestSunLight;

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
};