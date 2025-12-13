#version 450

layout (location = 0) out vec4 FragColor;
layout (location = 0) in vec3 Normal;
layout (location = 1) in vec2 TextureCoords;

struct DirectionalLight {
	vec3 direction;
	vec3 color;
	float intensity;
};

struct PointLight {
	vec3 position;
	vec3 color;
	float intensity;
};

layout (set = 0, binding = 0) uniform CameraInfo {
	mat4 View;
	mat4 Projection;
} camera;

layout (set = 0, binding = 1) uniform PointLightsData {
	int pointLightCount;
	PointLight pointLights[1024];
} pointLight;

layout (set = 0, binding = 2) uniform DirectionalLightData {
	DirectionalLight light;
} directionalLight;

layout (set = 1, binding = 0) uniform sampler2D DiffuseTexture;
layout (set = 1, binding = 1) uniform sampler2D RoughnessTexture;
layout (set = 1, binding = 2) uniform sampler2D NormalTexture;

const float AMBIENT = 0.05f;

void main() {
	vec4 a = texture(DiffuseTexture, TextureCoords);
	vec3 n = texture(NormalTexture, TextureCoords).rgb;
	float lightIntensity = AMBIENT + max(dot(Normal, directionalLight.light.direction), 0);
	FragColor = vec4(lightIntensity * a.rgb, a.a);
}