#include "Camera.h"
#include <limits>

#include <glm/gtc/matrix_transform.hpp>

void Camera::SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far)
{
	_ProjectionMatrix = glm::mat4{ 1.0f };
	_ProjectionMatrix[0][0] = 2.0f / (right - left);
	_ProjectionMatrix[1][1] = 2.0f / (bottom - top);
	_ProjectionMatrix[2][2] = 1.0f / (far - near);
	_ProjectionMatrix[3][0] = -(right + left) / (right - left);
	_ProjectionMatrix[3][1] = -(bottom + top) / (bottom - top);
	_ProjectionMatrix[3][2] = -near / (far - near);
}

void Camera::SetPerspectiveProjection(float fov, float aspect, float near, float far)
{
	ASSERT(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
	const float tanHalfFOV = tan(fov / 2.0f);
	_ProjectionMatrix = glm::mat4{ 0.0f };
	_ProjectionMatrix[0][0] = 1.0f / (aspect * tanHalfFOV);
	_ProjectionMatrix[1][1] = 1.0f / (tanHalfFOV);
	_ProjectionMatrix[2][2] = far  / (far - near);
	_ProjectionMatrix[2][3] = 1.0f;
	_ProjectionMatrix[3][2] = -(far * near) / (far - near);
}

void Camera::SetViewDirection(glm::vec3 position, glm::vec3 direction)
{
	const glm::vec3 w{ glm::normalize(direction) };
	const glm::vec3 u{ glm::normalize(glm::cross(w, _Up)) };
	const glm::vec3 v{ glm::cross(w, u) };

	_ViewMatrix = glm::mat4{ 1.f };
	_ViewMatrix[0][0] = u.x;
	_ViewMatrix[1][0] = u.y;
	_ViewMatrix[2][0] = u.z;
	_ViewMatrix[0][1] = v.x;
	_ViewMatrix[1][1] = v.y;
	_ViewMatrix[2][1] = v.z;
	_ViewMatrix[0][2] = w.x;
	_ViewMatrix[1][2] = w.y;
	_ViewMatrix[2][2] = w.z;
	_ViewMatrix[3][0] = -glm::dot(u, position);
	_ViewMatrix[3][1] = -glm::dot(v, position);
	_ViewMatrix[3][2] = -glm::dot(w, position);
}

void Camera::SetViewTarget(glm::vec3 position, glm::vec3 target)
{
	SetViewDirection(position, target - position);
}

void Camera::SetViewYXZ(glm::vec3 position, glm::vec3 rotation)
{
	const float c3 = glm::cos(rotation.z);
	const float s3 = glm::sin(rotation.z);
	const float c2 = glm::cos(rotation.x);
	const float s2 = glm::sin(rotation.x);
	const float c1 = glm::cos(rotation.y);
	const float s1 = glm::sin(rotation.y);
	const glm::vec3 u{ (c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1) };
	const glm::vec3 v{ (c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3) };
	const glm::vec3 w{ (c2 * s1), (-s2), (c1 * c2) };
	_ViewMatrix = glm::mat4{ 1.f };
	_ViewMatrix[0][0] = u.x;
	_ViewMatrix[1][0] = u.y;
	_ViewMatrix[2][0] = u.z;
	_ViewMatrix[0][1] = v.x;
	_ViewMatrix[1][1] = v.y;
	_ViewMatrix[2][1] = v.z;
	_ViewMatrix[0][2] = w.x;
	_ViewMatrix[1][2] = w.y;
	_ViewMatrix[2][2] = w.z;
	_ViewMatrix[3][0] = -glm::dot(u, position);
	_ViewMatrix[3][1] = -glm::dot(v, position);
	_ViewMatrix[3][2] = -glm::dot(w, position);
}