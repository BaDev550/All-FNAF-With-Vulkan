#pragma once

#include "Animatronic.h"

class Animatronic_Bonnie : public Animatronic {
public:
	Animatronic_Bonnie(SceneRenderer& renderer) :
		Animatronic(renderer,
			"Resources/Models/Cube/Cube.gltf",
			"Bonnie"
		) 
	{
		_Transform.scale = glm::vec3(0.2f, 0.7f, 0.2f);
		_PlacesItCanBe["MAIN_STAGE"] = { -1.0f, -0.5f, 10.1f };
		_PlacesItCanBe["BACK_STAGE"] = { -5.3f, -0.6f, 6.4f };
		_PlacesItCanBe["SUPPLY_CLOSET"] = { -2.5f, -0.6f, 2.1f };
		_PlacesItCanBe["LEFT_HALL_WINDOW"] = LEFT_HALL_WINDOW;
		_PlacesItCanBe["LEFT_HALL_BACK"] = LEFT_HALL_BACK;
		_PlacesItCanBe["LEFT_DOOR"] = LEFT_DOOR;
		_MovementPath = {
			"MAIN_STAGE",
			"BACK_STAGE",
			"LEFT_HALL_WINDOW",
			"LEFT_HALL_BACK",
			"SUPPLY_CLOSET",
			"LEFT_DOOR"
		};

		MoveTo("MAIN_STAGE");

		_AIAnimatronicStats.baseAggression = 6;
		_AIAnimatronicStats.baseMoveCooldown = 15.0f;
		_AIAnimatronicStats.baseAttackChance = 0.15f;
		_AIAnimatronicStats.baseMaxWaitAtDoor = 8.0f;
		_AIAnimatronicStats.baseAttackDelay = 3.5f;
	}
};