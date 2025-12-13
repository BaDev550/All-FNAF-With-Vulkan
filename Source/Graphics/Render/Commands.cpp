#include "gpch.h"
#include "Commands.h"
#include "Engine/Application.h"

void Commands::BeginRenderPass(VkCommandBuffer cmd)
{
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = Application::Get()->GetRenderer().GetSwapChainRenderPass();
	renderPassInfo.framebuffer = Application::Get()->GetRenderer().GetSwapChainCurrentFramebuffer();
	renderPassInfo.renderArea.offset = { 0, 0 };
	renderPassInfo.renderArea.extent = Application::Get()->GetRenderer().GetSwapChainExtent();
	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { 0.2f, 0.2f, 0.2f, 1.0f };
	clearValues[1].depthStencil = { 1.0f, 0 };
	renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
	renderPassInfo.pClearValues = clearValues.data();

	vkCmdBeginRenderPass(cmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Commands::EndRenderPass(VkCommandBuffer cmd)
{
	vkCmdEndRenderPass(cmd);
}

void Commands::BindDescriptorSet(VkCommandBuffer cmd, Pipeline& pipeline, uint32_t set, uint32_t binding, VkDescriptorSet data)
{
	vkCmdBindDescriptorSets(
		cmd,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline.GetLayout(),
		set, 1,
		&data,
		0, nullptr
	);
}

void Commands::SetViewport(VkCommandBuffer cmd, uint32_t w, uint32_t h)
{
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(w);
	viewport.height = static_cast<float>(h);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor{ {0,0}, Application::Get()->GetRenderer().GetSwapChainExtent() };
	vkCmdSetViewport(cmd, 0, 1, &viewport);
	vkCmdSetScissor(cmd, 0, 1, &scissor);
}

void DrawMesh(VkCommandBuffer cmd, Model& model, uint32_t meshIndex) {
	ASSERT(meshIndex < model.GetMeshes().size());
	const Mesh& mesh = model.GetMeshes()[meshIndex];

	vkCmdDrawIndexed(cmd,
		mesh.GetIndexCount(),
		1,
		mesh.GetIndexOffset(),
		mesh.GetVertexOffset(),
		0);
}

void Commands::DrawIndexed(VkCommandBuffer cmd, Model& model)
{
	model.Bind(cmd);
	for (size_t i = 0; i < model.GetMeshes().size(); ++i)
		DrawMesh(cmd, model, i);
}

void Commands::DrawMeshedModel(VkCommandBuffer cmd, Model& model, uint32_t meshIndex)
{
	model.Bind(cmd);
	DrawMesh(cmd, model, meshIndex);
}

void Commands::DrawMeshesWithMaterial(VkCommandBuffer cmd, Pipeline& pipeline, VkDescriptorSet fallbackTextureSet, Model& model, const glm::mat4& transform)
{
	model.Bind(cmd);

	auto& meshes = model.GetMeshes();
	VkPipelineLayout pipelineLayout = pipeline.GetLayout();

	vkCmdPushConstants(
		cmd,
		pipelineLayout,
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(glm::mat4),
		&transform
	);

	for (size_t meshIndex = 0; meshIndex < meshes.size(); meshIndex++) {
		auto& mesh = meshes[meshIndex];
		if (!mesh.IsVisible())
			continue;

		uint32_t materialId = mesh.GetMaterialId();

		VkDescriptorSet textureToBind = model.GetMaterial(materialId).GetDescriptorSet();
		if (textureToBind == VK_NULL_HANDLE)
			textureToBind = fallbackTextureSet;

		vkCmdBindDescriptorSets(
			cmd,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			1, 1,
			&textureToBind,
			0,
			nullptr
		);
		DrawMesh(cmd, model, meshIndex);
	}
}

void Commands::DrawVertex(VkCommandBuffer cmd, uint32_t vertexCount)
{
	vkCmdDraw(cmd, vertexCount, 1, 0, 0);
}
