#pragma once
#include <glm/glm.hpp>

class Camera
{
public:
	void SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
	void SetPerspectiveProjection(float fov, float aspect, float near, float far);
	void SetViewDirection(glm::vec3 position, glm::vec3 direction);
	void SetViewTarget(glm::vec3 position, glm::vec3 target);
	void SetViewYXZ(glm::vec3 position, glm::vec3 rotation);
	const glm::mat4& GetView() const { return _ViewMatrix; }
	const glm::mat4& GetProjection() const { return _ProjectionMatrix; }
private:
	glm::mat4 _ProjectionMatrix{ 1.0f };
	glm::mat4 _ViewMatrix{ 1.0f };

	glm::vec3 _Up = { 0.0f, -1.0f, 0.0f };
};

