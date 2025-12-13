#pragma once
#include "Graphics/Render/Descriptor.h"
#include "Graphics/Render/Renderer.h"
#include "Graphics/Model/Texture.h"
#include "Graphics/Render/Light.h"
#include "Graphics/Camera.h"
#include "Objects/GameObject.h"
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
};

struct SceneInfo {
	SceneLightEnviroment lightEnviroment;

	UniformBufferDirectionalLight uboDirectionalLight;
	UniformBufferPointLights uboPointLights;

	MEM::Scope<DescriptorPool> scenePool = nullptr;
	MEM::Ref<Texture> fallbackTexture = nullptr;
	VkDescriptorSet fallbackTextureSet = VK_NULL_HANDLE;
	VkDescriptorSet generalInfoSet = VK_NULL_HANDLE;

	MEM::Scope<DescriptorSetLayout> generalSetLayout = nullptr;
	MEM::Scope<DescriptorSetLayout> textureSetLayout = nullptr;

	CameraInfo cameraInfo{};
	MEM::Scope<Buffer> cameraUniformBuffer = nullptr;
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
	void CreateDrawnable(GameObject& object, const std::string& path);
	Drawnable& GetDrawnable(gameobjectid_t id);

	void Draw(Pipeline& pipeline, std::vector<GameObject>& objects, Camera& camera, std::function<void(VkCommandBuffer)> overlayCallback = nullptr);
private:
	Renderer& _Renderer;
	SceneInfo _SceneInfo;
	std::vector<Drawnable> _Drawnables;
};

