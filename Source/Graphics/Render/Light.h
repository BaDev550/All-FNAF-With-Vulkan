#pragma once

#include "glm/glm.hpp"
#define MAX_DIRECTIONAL_LIGHTS 1
#define MAX_POINT_LIGHTS 1024

struct DirectionalLight {
	glm::vec3 Direction;
	alignas(16) glm::vec3 Color;
	float Intensity;

	DirectionalLight() : Direction(0.0f, -1.0f, 0.0f), Color(1.0f), Intensity(1.0f) {}
};

struct PointLight {
	glm::vec3 Position;
	alignas(16) glm::vec3 Color;
	float Intensity;
	
	PointLight() : Position(0.0f), Color(1.0f), Intensity(1.0f) {}
};

struct UniformBufferDirectionalLight
{
	DirectionalLight directionalLight;
};

struct UniformBufferPointLights
{
	int Count{ 0 };
	int _padding[3];
	PointLight PointLights[MAX_POINT_LIGHTS]{};
};