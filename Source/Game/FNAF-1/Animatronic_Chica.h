#pragma once

#include "Animatronic.h"

class Animatronic_Chica : public Animatronic {
public:
	Animatronic_Chica(SceneRenderer& renderer) :
		Animatronic(renderer,
			"Resources/Models/Cube/Cube.gltf",
			"Chica"
		) 
	{
		_Transform.scale = glm::vec3(0.2f, 0.7f, 0.2f);
		_PlacesItCanBe["MAIN_STAGE"] = { 1.0f, -0.5f, 10.1f };
		_PlacesItCanBe["RIGHT_HALL_WINDOW"] = RIGHT_HALL_WINDOW;
		_PlacesItCanBe["RIGHT_HALL_BACK"] = RIGHT_HALL_BACK;
		_PlacesItCanBe["RIGHT_DOOR"] = RIGHT_DOOR;
		_MovementPath = {
			"MAIN_STAGE",
			"RIGHT_HALL_WINDOW",
			"RIGHT_HALL_BACK",
			"RIGHT_DOOR"
		};

		MoveTo("MAIN_STAGE");

		_AIAnimatronicStats.baseAggression = 5;
		_AIAnimatronicStats.baseMoveCooldown = 16.0f;
		_AIAnimatronicStats.baseAttackChance = 0.15f;
		_AIAnimatronicStats.baseMaxWaitAtDoor = 8.0f;
		_AIAnimatronicStats.baseAttackDelay = 4.5f;
	}
};