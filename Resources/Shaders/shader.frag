#version 450

layout (location = 0) out vec4 FragColor;
layout (location = 0) in vec3 Normal;
layout (location = 1) in vec2 TextureCoords;
layout (location = 2) in vec4 WorldPos;

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
	vec3 cameraPos;
} camera;

layout (set = 0, binding = 1) uniform PointLightsData {
	int pointLightCount;
	PointLight lights[100];
} pointLight;

layout (set = 0, binding = 2) uniform DirectionalLightData {
	DirectionalLight light;
} directionalLight;

layout (set = 1, binding = 0) uniform sampler2D DiffuseTexture;
layout (set = 1, binding = 1) uniform sampler2D RoughnessTexture;
layout (set = 1, binding = 2) uniform sampler2D NormalTexture;

const float AMBIENT = 0.00025f;
const vec3 FNAF_TINT = vec3(0.65, 0.75, 0.85);
const float FOG_DENSITY = 0.6f;

void main() {
	vec4 albedo = texture(DiffuseTexture, TextureCoords);
	vec3 N = normalize(Normal);
	vec3 V = normalize(camera.cameraPos - WorldPos.xyz);

	float dirLight = AMBIENT + max(dot(N, -normalize(directionalLight.light.direction)), 0.0) * directionalLight.light.intensity * 0.15;
	vec3 diffuseLight = vec3(0.0);
	for (int i = 0; i < pointLight.pointLightCount; i++) {
		vec3 L = pointLight.lights[i].position - WorldPos.xyz;
		float dist = length(L);
		vec3 Ldir = normalize(L);
		float attenuation = 1.0 / (dist * dist * dist);

		float NdotL = max(dot(N, Ldir), 0.0);

		diffuseLight +=
			pointLight.lights[i].color *
			pointLight.lights[i].intensity *
			NdotL *
			attenuation;
	}
	vec3 lighting = diffuseLight + vec3(dirLight);

	lighting = min(lighting, vec3(1.0));
	lighting *= FNAF_TINT;

	float distance = length(camera.cameraPos - WorldPos.xyz);
	float fogFactor = exp(-FOG_DENSITY * distance);
	fogFactor = clamp(fogFactor, 0.0, 1.0);

	vec3 finalColor = albedo.rgb * lighting;
	finalColor = mix(vec3(0.0), finalColor, fogFactor);

	float gray = dot(finalColor, vec3(0.299, 0.587, 0.114));
	finalColor = mix(vec3(gray), finalColor, 0.6);

	FragColor = vec4(finalColor, albedo.a);
}