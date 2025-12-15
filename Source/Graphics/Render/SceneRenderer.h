#pragma once
#include "Graphics/Render/Descriptor.h"
#include "Graphics/Render/Renderer.h"
#include "Graphics/Model/Texture.h"
#include "Graphics/Render/Light.h"
#include "Graphics/Camera.h"
#include "Objects/GameObject.h"
#include "Objects/CameraObject.h"
#include "Commands.h"

constexpr uint32_t MAX_MESHES_PER_OBJECT = 400;

struct Drawnable {
	gameobjectid_t ownerId;
	MEM::Ref<Model> model;
	bool IsVisible = true;
};

struct SceneLightEnviroment {
	DirectionalLight DirectionalLight[MAX_DIRECTIONAL_LIGHTS];
	std::vector<PointLight> PointLights;
	_NODISCARD uint32_t GetPointLightCount() const { return (uint32_t)(PointLights.size()); }
};

struct CameraInfo {
	glm::mat4 view;
	glm::mat4 projection;
	alignas(16) glm::vec3 position;
};

struct SceneInfo {
	SceneLightEnviroment lightEnviroment;

	UniformBufferDirectionalLight uboDirectionalLight;
	UniformBufferPointLights uboPointLights;

	MEM::Ref<Texture> fallbackTexture = nullptr;
	MEM::Scope<DescriptorPool> scenePool = nullptr;
	std::vector<VkDescriptorSet> generalInfoSets;
	VkDescriptorSet fallbackTextureSet = VK_NULL_HANDLE;

	MEM::Scope<DescriptorSetLayout> generalSetLayout = nullptr;
	MEM::Scope<DescriptorSetLayout> textureSetLayout = nullptr;

	CameraInfo cameraInfo{};
	std::vector<MEM::Scope<Buffer>> cameraBuffers;
	MEM::Scope<Buffer> directionalLightBuffer = nullptr;
	MEM::Scope<Buffer> pointLightBuffer = nullptr;

	std::vector<VkDescriptorSetLayout> GetLayoutHandles() const {
		return {
			generalSetLayout->GetDescriptorSetLayout(),
			textureSetLayout->GetDescriptorSetLayout()
		};
	}
};

class SceneRenderer
{
public:
	SceneRenderer(Renderer& renderer);
	~SceneRenderer();
	SceneInfo& GetSceneInfo() { return _SceneInfo; }
	Drawnable& CreateDrawnable(GameObject& object, const std::string& path);
	Drawnable& GetDrawnable(gameobjectid_t id);

	void Draw(VkCommandBuffer cmd, Pipeline& pipeline, std::vector<MEM::Ref<GameObject>>& objects, CameraObject& camera, uint32_t viewIndex = 0);
private:
	Renderer& _Renderer;
	SceneInfo _SceneInfo;
	std::vector<Drawnable> _Drawnables;
};

