#pragma once
#include <vulkan/vulkan.h>
#include <iostream>

#include "Graphics/Model/Model.h"
#include "Graphics/Pipeline/Pipeline.h"

class Commands {
public:
	static void BeginRenderPass(VkCommandBuffer cmd);
	static void EndRenderPass(VkCommandBuffer cmd);
	static void BindDescriptorSet(VkCommandBuffer cmd, Pipeline& pipeline, uint32_t set, uint32_t binding, VkDescriptorSet data);
	static void SetViewport(VkCommandBuffer cmd, uint32_t w, uint32_t h);
	static void DrawIndexed(VkCommandBuffer cmd, Model& model);
	static void DrawMeshedModel(VkCommandBuffer cmd, Model& model, uint32_t meshIndex);
	static void DrawMeshesWithMaterial(VkCommandBuffer cmd, Pipeline& pipeline, VkDescriptorSet fallbackTextureSet, Model& model, const glm::mat4& transform);
	static void DrawVertex(VkCommandBuffer cmd, uint32_t vertexCount);
};