#pragma once

#include "Objects/GameObject.h"
#include "Audio/SoundBuffer.h"
#include "Audio/SoundSource.h"
#include <string>

enum class AnimatronicState : uint8_t {
	IDLE,
	MOVING,
	AT_DOOR,
	ATTACKING
};

enum class DoorSide : uint8_t {
	NONE,
	LEFT,
	RIGHT
};

struct AnimatronicAIStats {
	int baseAggression;
	float baseMoveCooldown;
	float baseAttackChance;
	float baseMaxWaitAtDoor;
	float baseAttackDelay;

	int aggression = 0;
	float moveCooldown = 5.0f;
	float moveTimer = 0.0f;
	float attackChance = 0.0f;
	float maxWaitAtDoor = 10.0f;
	float attackDelay = 5.0f;
};

constexpr glm::vec3 RIGHT_DOOR = { 0.9f, -0.2f, -0.3f };
constexpr glm::vec3 LEFT_DOOR = { -0.9f, -0.2f, -0.3f };
constexpr glm::vec3 RIGHT_HALL_WINDOW = { 1.2f, -0.5f, 1.2f };
constexpr glm::vec3 RIGHT_HALL_BACK = { 1.2f, -0.5f, -1.2f };
constexpr glm::vec3 LEFT_HALL_WINDOW = { -1.2f, -0.5f, 1.2f };
constexpr glm::vec3 LEFT_HALL_BACK = { -1.2f, -0.5f, -1.2f };

class Animatronic : public DrawnableGameObject {
public:
	Animatronic(SceneRenderer& renderer, const std::string& modelPath, const std::string& name) 
		: DrawnableGameObject(renderer, modelPath, currentId++, name) {
		_DeepStepsSoundEffect.SetBuffer(_AnimatronicSoundEffectsBuffer.AddSoundEffect("Resources/Audio/deep steps.wav"));
		_JumpscareSoundEffect.SetBuffer(_AnimatronicSoundEffectsBuffer.AddSoundEffect("Resources/Audio/XSCREAM.wav"));
	}
	using DrawnableGameObject::DrawnableGameObject;

	void Update(float dt);
	void MoveTo(const std::string& where);
	void TryRetreat();

	void SetNightDifficulty(int night);
	AnimatronicState GetState() const { return _AIAnimatronicState; }
	DoorSide GetDoorSide() const { return _DoorSide; }
	float GetTimeAtDoor() const { return _TimeAtDoor; }
	float GetMaxTimeAtDoor() const { return _AIAnimatronicStats.maxWaitAtDoor; }
	void AddAttackTime(float dt) { _AttackTimer += dt; }
	float GetAttackTime() const { return _AttackTimer; }
	float GetAttackDelay() const { return _AIAnimatronicStats.attackDelay; }
	void ResetAttackTimer() { _AttackTimer = 0.0f; }
	bool AnimatronicMovedThisFrame() { return _MovedThisFrame; }
	void ResetMovedState() { _MovedThisFrame = false; }
	void SetNightProgress(float progress) { _PlayerNightProgress = progress; }

	void Jumpscare();
protected:
	std::unordered_map<std::string, glm::vec3> _PlacesItCanBe = {};
	std::vector<std::string> _MovementPath;

	float _TimeAtDoor = 0.0f;
	float _AttackTimer = 0.0f;
	float _PlayerNightProgress = 0.0f;
	int _CurrentPathIndex = 0;
	bool _MovedThisFrame = false;
	DoorSide _DoorSide = DoorSide::NONE;
	AnimatronicState _AIAnimatronicState = AnimatronicState::IDLE;
	AnimatronicAIStats _AIAnimatronicStats;

	void SetAtDoor(DoorSide side);
	virtual void DecideNextMove();

	SoundBuffer _AnimatronicSoundEffectsBuffer;
	SoundSource _DeepStepsSoundEffect;
	SoundSource _JumpscareSoundEffect;
};