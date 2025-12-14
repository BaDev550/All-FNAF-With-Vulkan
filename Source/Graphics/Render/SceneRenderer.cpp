#include "gpch.h"
#include "SceneRenderer.h"
#include "Engine/Application.h"

SceneRenderer::SceneRenderer(Renderer& renderer)
	: _Renderer(renderer)
{
	_SceneInfo.scenePool = DescriptorPool::Builder()
		.SetMaxSets(1000)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2048)
		.Build();

	_SceneInfo.generalSetLayout = DescriptorSetLayout::Builder()
		.AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
		.AddBinding(1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.AddBinding(2, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.Build();

	_SceneInfo.textureSetLayout = DescriptorSetLayout::Builder()
		.AddBinding(DIFFUSE_TEXTURE_BINDING, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.AddBinding(ROUGHNESS_TEXTURE_BINDING, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.AddBinding(NORMAL_TEXTURE_BINDING, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
		.Build();

	_SceneInfo.fallbackTexture = MEM::CreateRef<Texture>("Resources/Textures/Engine/missing.png");
	VkDescriptorImageInfo fallbackImageInfo = _SceneInfo.fallbackTexture->DescriptorInfo();
	DescriptorWriter(*_SceneInfo.textureSetLayout, *_SceneInfo.scenePool)
		.WriteImage(DIFFUSE_TEXTURE_BINDING, &fallbackImageInfo)
		.Build(_SceneInfo.fallbackTextureSet);

	_SceneInfo.pointLightBuffer = MEM::CreateScope<Buffer>(
		sizeof(UniformBufferPointLights),
		1,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		Application::Get()->GetDevice().GetDeviceProperties().limits.minUniformBufferOffsetAlignment
	);
	_SceneInfo.pointLightBuffer->Map();

	_SceneInfo.directionalLightBuffer = MEM::CreateScope<Buffer>(
		sizeof(UniformBufferDirectionalLight),
		1,
		VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		Application::Get()->GetDevice().GetDeviceProperties().limits.minUniformBufferOffsetAlignment
	);
	_SceneInfo.directionalLightBuffer->Map();
	VkDescriptorBufferInfo pointLightInfo = _SceneInfo.pointLightBuffer->DescriptorInfo();
	VkDescriptorBufferInfo dirLightInfo = _SceneInfo.directionalLightBuffer->DescriptorInfo();

	_SceneInfo.cameraBuffers.resize(2);
	_SceneInfo.generalInfoSets.resize(2);
	for (int i = 0; i < 2; i++) {
		_SceneInfo.cameraBuffers[i] = MEM::CreateScope<Buffer>(
			sizeof(CameraInfo),
			1,
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
			Application::Get()->GetDevice().GetDeviceProperties().limits.minUniformBufferOffsetAlignment
		);
		_SceneInfo.cameraBuffers[i]->Map();

		VkDescriptorBufferInfo camInfo = _SceneInfo.cameraBuffers[i]->DescriptorInfo();
		DescriptorWriter(*_SceneInfo.generalSetLayout, *_SceneInfo.scenePool)
			.WriteBuffer(0, &camInfo)
			.WriteBuffer(1, &pointLightInfo)
			.WriteBuffer(2, &dirLightInfo)
			.Build(_SceneInfo.generalInfoSets[i]);
	}
}

SceneRenderer::~SceneRenderer()
{
}

Drawnable& SceneRenderer::CreateDrawnable(GameObject& object, const std::string& path)
{
	Drawnable d{};
	d.ownerId = object.GetID();
	d.model = Model::CreateModelFromFile(path, _SceneInfo.fallbackTexture->DescriptorInfo(), *_SceneInfo.scenePool, *_SceneInfo.textureSetLayout);
	d.IsVisible = true;
	_Drawnables.push_back(d);
	return _Drawnables.back();
}

Drawnable& SceneRenderer::GetDrawnable(gameobjectid_t id)
{
	for (auto& drawnable : _Drawnables) {
		if (drawnable.ownerId == id)
			return drawnable;
	}
}

void SceneRenderer::Draw(VkCommandBuffer cmd, Pipeline& pipeline, std::vector<GameObject>& objects, Camera& camera, uint32_t viewIndex)
{
	if (viewIndex >= _SceneInfo.cameraBuffers.size()) return;

	_SceneInfo.cameraInfo.view = camera.GetView();
	_SceneInfo.cameraInfo.projection = camera.GetProjection();
	_SceneInfo.cameraBuffers[viewIndex]->WriteToBuffer(&_SceneInfo.cameraInfo);

	auto& pointLightsSrc = _SceneInfo.lightEnviroment.PointLights;
	auto& uboPointsDest = _SceneInfo.uboPointLights;
	uboPointsDest.Count = static_cast<int>(pointLightsSrc.size());
	if (uboPointsDest.Count > MAX_POINT_LIGHTS) uboPointsDest.Count = MAX_POINT_LIGHTS;

	if (uboPointsDest.Count > 0) {
		std::memcpy(uboPointsDest.PointLights, pointLightsSrc.data(), sizeof(PointLight) * uboPointsDest.Count);
		_SceneInfo.pointLightBuffer->WriteToBuffer(&uboPointsDest);
	}

	_SceneInfo.uboDirectionalLight.directionalLight = _SceneInfo.lightEnviroment.DirectionalLight[0];
	_SceneInfo.directionalLightBuffer->WriteToBuffer(&_SceneInfo.uboDirectionalLight);

	pipeline.Bind(cmd);
	Commands::BindDescriptorSet(cmd, pipeline, 0, 1, _SceneInfo.generalInfoSets[viewIndex]);
	for (auto& drawnable : _Drawnables) {
		if (drawnable.IsVisible) {
			glm::mat4 transform = objects[drawnable.ownerId].GetTransform().mat4();

			Commands::DrawMeshesWithMaterial(
				cmd,
				pipeline,
				_SceneInfo.fallbackTextureSet,
				*drawnable.model,
				transform
			);
		}
	}
}
