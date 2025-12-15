#version 450
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;

layout (location = 0) out vec3 Normal;
layout (location = 1) out vec2 TextureCoords;
layout (location = 2) out vec4 WorldPos;

layout (set = 0, binding = 0) uniform CameraInfo {
	mat4 View;
	mat4 Projection;
} camera;

layout(push_constant) uniform Push {
	mat4 Model;
} push;

void main() {
	vec4 worldPos = push.Model * vec4(aPosition, 1.0f);
	Normal = normalize(aNormal);
	TextureCoords = aTexCoords;
	WorldPos = worldPos;
	gl_Position = camera.Projection * camera.View * worldPos;
}