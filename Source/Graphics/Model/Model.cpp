#include "gpch.h"
#include "Model.h"
#include "Utils/HashCombiner.h"
#include "Utils/AssimpToGLM.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <unordered_map>
#include <map>
#include <filesystem>
#include "Engine/Application.h"

namespace std {
	template<>
	struct hash<Vertex> {
		size_t operator()(Vertex const& vertex) const {
			size_t seed = 0;
			HashCombine(seed, vertex.Position, vertex.TextureCoords, vertex.Normal);
			return seed;
		}
	};
}

static const uint32_t S_ASSIMPIMPORTERFLAGS =
	aiProcess_Triangulate |
	aiProcess_GenNormals |
	aiProcess_FlipUVs |
	aiProcess_JoinIdenticalVertices;

MEM::Ref<Model> Model::CreateModelFromFile(const std::string& path, VkDescriptorImageInfo fallbackTextureInfo, DescriptorPool& pool, DescriptorSetLayout& layout)
{
	auto model = MEM::CreateRef<Model>();
	model->LoadModelFromFile(path, fallbackTextureInfo, pool, layout);
	return std::move(model);
}

void Model::Bind(VkCommandBuffer commandBuffer) {
	VkBuffer buffers[] = { _VertexBuffer->GetBuffer() };
	VkDeviceSize offsets[] = { 0 };
	vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

	if (_HasIndexBuffer)
		vkCmdBindIndexBuffer(commandBuffer, _IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
}

void Model::CreateVertexBuffer(const std::vector<Vertex>& vertices) {
	_Vertices = vertices;
	_VertexCount = static_cast<uint32_t>(vertices.size());
	assert(_VertexCount >= 3 && "Vertex count must be at least 3");

	VkDeviceSize bufferSize = sizeof(vertices[0]) * _VertexCount;
	uint32_t vertexSize = sizeof(vertices[0]);
	Buffer stagingBuffer{
		vertexSize,
		_VertexCount,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	};
	stagingBuffer.Map();
	stagingBuffer.WriteToBuffer((void*)vertices.data());

	_VertexBuffer = MEM::CreateScope<Buffer>(
		vertexSize,
		_VertexCount,
		VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);
	Application::Get()->GetDevice().CopyBuffer(stagingBuffer.GetBuffer(), _VertexBuffer->GetBuffer(), bufferSize);
}

void Model::CreateIndexBuffer(const std::vector<uint32_t>& indices)
{
	_Indices = indices;
	_IndexCount = static_cast<uint32_t>(indices.size());
	_HasIndexBuffer = _IndexCount > 0;
	if (!_HasIndexBuffer)
		return;

	VkDeviceSize bufferSize = sizeof(indices[0]) * _IndexCount;
	uint32_t indexSize = sizeof(indices[0]);
	Buffer stagingBuffer{
		indexSize,
		_IndexCount,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
	};
	stagingBuffer.Map();
	stagingBuffer.WriteToBuffer((void*)indices.data());

	_IndexBuffer = MEM::CreateScope<Buffer>(
		indexSize,
		_IndexCount,
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);
	Application::Get()->GetDevice().CopyBuffer(stagingBuffer.GetBuffer(), _IndexBuffer->GetBuffer(), bufferSize);
}

std::vector<VkVertexInputBindingDescription> Vertex::GetBindingDescriptions()
{
	std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
	bindingDescriptions[0].binding = 0;
	bindingDescriptions[0].stride = sizeof(Vertex);
	bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription> Vertex::GetAttributeDescriptions()
{
	std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
	attributeDescriptions.push_back({ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Position) });
	attributeDescriptions.push_back({ 1, 0, VK_FORMAT_R32G32_SFLOAT,    offsetof(Vertex, TextureCoords) });
	attributeDescriptions.push_back({ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, Normal) });
	return attributeDescriptions;
}

MEM::Ref<Texture> Model::LoadMaterialTexture(const std::string& path, std::unordered_map<std::string, MEM::Ref<Texture>>& cache) {
	if (cache.find(path) != cache.end()) {
		return cache[path];
	}

	try {
		MEM::Ref<Texture> newTex = MEM::CreateRef<Texture>(path);
		cache[path] = newTex;
		return newTex;
	}
	catch (...) {
		LOG_WARN("Failed to load texture: {}", path);
		return nullptr;
	}
}

void Model::ProcessMaterials(const aiScene* scene, const std::string& modelPath, VkDescriptorImageInfo fallbackTextureInfo, DescriptorPool& pool, DescriptorSetLayout& layout) {
	std::filesystem::path rootPath = std::filesystem::path(modelPath).parent_path();
	std::unordered_map<std::string, MEM::Ref<Texture>> textureCache;

	for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
		aiMaterial* aiMat = scene->mMaterials[i];
		Material material;
		material.SetId(i);

		auto tryLoadTexture = [&](aiTextureType type) -> MEM::Ref<Texture> {
			if (aiMat->GetTextureCount(type) > 0) {
				aiString str;
				aiMat->GetTexture(type, 0, &str);
				std::string fullPath = (rootPath / str.C_Str()).string();
				return LoadMaterialTexture(fullPath, textureCache);
			}
			return nullptr;
			};

		material._DiffuseTexture = tryLoadTexture(aiTextureType_DIFFUSE);
		material._RoughnessTexture = tryLoadTexture(aiTextureType_DIFFUSE_ROUGHNESS);
		material._NormalTexture = tryLoadTexture(aiTextureType_NORMALS);

		VkDescriptorImageInfo diffuseInfo = material._DiffuseTexture ? material._DiffuseTexture->DescriptorInfo() : fallbackTextureInfo;
		VkDescriptorImageInfo roughnessInfo = material._RoughnessTexture ? material._RoughnessTexture->DescriptorInfo() : fallbackTextureInfo;
		VkDescriptorImageInfo normalInfo = material._NormalTexture ? material._NormalTexture->DescriptorInfo() : fallbackTextureInfo;

		DescriptorWriter writer(layout, pool);
		writer.WriteImage(DIFFUSE_TEXTURE_BINDING, &diffuseInfo);
		writer.WriteImage(ROUGHNESS_TEXTURE_BINDING, &roughnessInfo);
		writer.WriteImage(NORMAL_TEXTURE_BINDING, &normalInfo);

		if (!writer.Build(material._MaterialSet)) {
			LOG_ERROR("Failed to create descriptor set for material {}", i);
		}

		_Materials[i] = material;
	}
}

void Model::LoadModelFromFile(const std::string& path, VkDescriptorImageInfo fallbackTextureInfo, DescriptorPool& pool, DescriptorSetLayout& layout)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path, S_ASSIMPIMPORTERFLAGS);
	if (!scene || !scene->mRootNode) {
		LOG_ERROR("Failed to load model: {}", path);
		return;
	}
	ProcessMaterials(scene, path, fallbackTextureInfo, pool, layout);

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;

	std::function<void(aiNode*)> processNode = [&](aiNode* node) {
		for (uint32_t i = 0; i < node->mNumMeshes; i++) {
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			uint32_t vertexOffset = static_cast<uint32_t>(vertices.size());
			uint32_t indexOffset = static_cast<uint32_t>(indices.size());

			for (uint32_t v = 0; v < mesh->mNumVertices; v++) {
				Vertex vertex{};
				vertex.Position = AiToGlm(mesh->mVertices[v]);
				vertex.Normal = AiToGlm(mesh->mNormals[v]);
				if (mesh->mTextureCoords[0]) {
					vertex.TextureCoords = AiToGlm(mesh->mTextureCoords[0][v]);
				}

				vertices.push_back(vertex);
			}

			for (uint32_t f = 0; f < mesh->mNumFaces; f++) {
				aiFace face = mesh->mFaces[f];
				for (uint32_t j = 0; j < face.mNumIndices; j++)
					indices.push_back(face.mIndices[j]);
			}

			Mesh newMesh(mesh->mName.C_Str());
			newMesh.SetVertexOffset(vertexOffset);
			newMesh.SetIndexOffset(indexOffset);
			newMesh.SetIndexCount(static_cast<uint32_t>(indices.size()) - indexOffset);
			newMesh.SetMaterialId(mesh->mMaterialIndex);
			_Meshes.push_back(newMesh);
		}

		for (uint32_t i = 0; i < node->mNumChildren; i++)
			processNode(node->mChildren[i]);
		};
	processNode(scene->mRootNode);
	CreateVertexBuffer(vertices);
	CreateIndexBuffer(indices);
}