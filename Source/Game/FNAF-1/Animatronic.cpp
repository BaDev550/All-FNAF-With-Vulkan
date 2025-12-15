#include "gpch.h"
#include "Animatronic.h"

void Animatronic::Update(float dt) {
    if (_PlayerNightProgress < 0.15f)
        return;

    if (_AIAnimatronicState == AnimatronicState::AT_DOOR) {
        _TimeAtDoor += dt;
        return;
    }

    float speedMultiplier = glm::mix(0.3f, 1.0f, _PlayerNightProgress);
    int effectiveAggression = _AIAnimatronicStats.aggression + static_cast<int>(_PlayerNightProgress);

    _AIAnimatronicStats.moveTimer += dt * speedMultiplier;
    if (_AIAnimatronicStats.moveTimer < _AIAnimatronicStats.moveCooldown)
        return;
    _AIAnimatronicStats.moveTimer = 0.0f;

    int roll = rand() % 20;
    if (roll >= effectiveAggression)
        return;
    DecideNextMove();
}

void Animatronic::MoveTo(const std::string& where)
{
    _Transform.position = _PlacesItCanBe[where];

    if (where == "RIGHT_DOOR")
        SetAtDoor(DoorSide::RIGHT);
    else if (where == "LEFT_DOOR")
        SetAtDoor(DoorSide::LEFT);
    else
        _AIAnimatronicState = AnimatronicState::MOVING;
    _MovedThisFrame = true;
}

void Animatronic::TryRetreat()
{
    int roll = rand() % 100;
    int retreatChance = 30 - _AIAnimatronicStats.aggression;
    if (roll < retreatChance) {
        _CurrentPathIndex = std::max(0, _CurrentPathIndex - 2);
        _AIAnimatronicState = AnimatronicState::MOVING;
        _DoorSide = DoorSide::NONE;
        _TimeAtDoor = 0.0f;
        _DeepStepsSoundEffect.Play();
        MoveTo(_MovementPath[_CurrentPathIndex]);
    }
}

void Animatronic::SetNightDifficulty(int night)
{
    float nightFactor = 1.0f + (night - 1) * 0.15f;

    _AIAnimatronicStats.aggression = std::min(20, int(_AIAnimatronicStats.baseAggression * nightFactor));
    _AIAnimatronicStats.moveCooldown = std::max(1.0f, _AIAnimatronicStats.baseMoveCooldown - night * 0.4f);
    _AIAnimatronicStats.attackChance = glm::clamp(_AIAnimatronicStats.baseAttackChance + night * 0.03f, 0.0f, 0.95f);
    _AIAnimatronicStats.maxWaitAtDoor = std::max(2.0f, _AIAnimatronicStats.baseMaxWaitAtDoor - night * 0.7f);
    _AIAnimatronicStats.attackDelay = std::max(0.8f, _AIAnimatronicStats.baseAttackDelay - night * 0.2f);
}

void Animatronic::Jumpscare()
{
    _JumpscareSoundEffect.Play();
}

void Animatronic::SetAtDoor(DoorSide side)
{
    _AIAnimatronicState = AnimatronicState::AT_DOOR;
    _DoorSide = side;
    _DeepStepsSoundEffect.Play();
}

void Animatronic::DecideNextMove()
{
    if (_CurrentPathIndex + 1 >= _MovementPath.size())
        return;
    _CurrentPathIndex++;

    MoveTo(_MovementPath[_CurrentPathIndex]);
}
