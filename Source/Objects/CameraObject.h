#pragma once

#include "GameObject.h"
#include <set>
#include <glm/glm.hpp>
#include "Graphics/Camera.h"

class CameraObject : public GameObject {
public:
	CameraObject() = default;
	CameraObject(const std::string& name, glm::vec3 position, glm::vec3 rotation) {
		_Name = name;
		GetTransform().position = position;
		GetTransform().rotation = rotation;
	}

	void Update();
	Camera& GetCamera() { return _Handle; }
private:
	Camera _Handle;
};