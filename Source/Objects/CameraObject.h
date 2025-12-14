#pragma once

#include "GameObject.h"
#include <set>
#include <glm/glm.hpp>
#include "Graphics/Camera.h"
#include "Engine/Application.h"

class CameraObject : public GameObject {
public:
	CameraObject() = default;
	CameraObject(const std::string& name, glm::vec3 position, glm::vec3 rotation) {
		_Name = name;
		GetTransform().position = position;
		GetTransform().rotation = rotation;
	}

	void Update() {
		_Handle.SetPerspectiveProjection(45.0f, Application::Get()->GetRenderer().GetAspectRatio(), 0.1f, 100.0f);
		_Handle.SetViewYXZ(GetTransform().position, GetTransform().rotation);
	}
	Camera& GetCamera() { return _Handle; }
private:
	Camera _Handle;
};