#pragma once
using renderid_t = unsigned int;
using gameobjectid_t = unsigned int;

#include <glm/gtc/matrix_transform.hpp>
#include <memory>

struct TransformComponent {
	glm::vec3 position = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

	const glm::mat4 mat4() {
		glm::mat4 result = glm::translate(glm::mat4(1.0f), position);
		result = glm::rotate(result, glm::radians(rotation.x), glm::vec3(1, 0, 0));
		result = glm::rotate(result, glm::radians(rotation.y), glm::vec3(0, 1, 0));
		result = glm::rotate(result, glm::radians(rotation.z), glm::vec3(0, 0, 1));
		result = glm::scale(result, scale);
		return result;
	}
};

class GameObject
{
public:
	GameObject(gameobjectid_t objID, std::string name) : _Id(objID), _Name(name) {}
	GameObject() : _Id(0), _Name("OBJECT") {}

	static GameObject CreateGameObject(std::string name) { // to-do make game class
		static gameobjectid_t currentId = 0;
		return GameObject{ currentId++, name };
	}
	TransformComponent& GetTransform() { return _Transform; }
	const std::string& GetName() const { return _Name; }

	gameobjectid_t GetID() const { return _Id; }
private:
	std::string _Name;
	gameobjectid_t _Id;
	renderid_t _RenderId = UINT32_MAX;

	TransformComponent _Transform;
};

