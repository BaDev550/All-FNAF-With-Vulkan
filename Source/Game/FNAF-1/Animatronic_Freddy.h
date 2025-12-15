#pragma once

#include "Animatronic.h"

class Animatronic_Freddy : public Animatronic {
public:
	Animatronic_Freddy(SceneRenderer& renderer) :
		Animatronic(renderer,
			"Resources/Models/Cube/Cube.gltf",
			"Freddy"
		) 
	{
		_Transform.scale = glm::vec3(0.2f, 0.7f, 0.2f);
		_PlacesItCanBe["MAIN_STAGE"] = { 0.0f, -0.5f, 9.1f };
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

		_AIAnimatronicStats.baseAggression = 2;
		_AIAnimatronicStats.baseMoveCooldown = 20.0f;
		_AIAnimatronicStats.baseAttackChance = 0.05f;
		_AIAnimatronicStats.baseMaxWaitAtDoor = 12.0f;
		_AIAnimatronicStats.baseAttackDelay = 1.8f;
	}
};