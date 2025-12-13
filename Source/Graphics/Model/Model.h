#pragma once
#include "Graphics/Buffers/Buffer.h"
#include "Graphics/Render/Descriptor.h"
#include "Texture.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <iostream>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <assimp/types.h>
#include <glm/glm.hpp>

constexpr uint32_t DIFFUSE_TEXTURE_BINDING = 0;
constexpr uint32_t ROUGHNESS_TEXTURE_BINDING = 1;
constexpr uint32_t NORMAL_TEXTURE_BINDING = 2;

enum TextureType {
	DIFFUSE = 0,
	ROUGHNESS = 1,
	NORMAL = 2
};

struct Vertex {
	glm::vec3 Position;
	glm::vec2 TextureCoords;
	glm::vec3 Normal;
	static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
	static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

	bool operator==(const Vertex& other) const { return Position == other.Position && TextureCoords == other.TextureCoords && Normal == other.Normal; }
};

class Material {
public:
	void SetId(uint32_t id) { _Id = id; }
	uint32_t GetId() const { return _Id; }
	bool IsValidMaterial() const { return _Id != UINT32_MAX; }

	VkDescriptorSet GetDescriptorSet() const { return _MaterialSet; }
	void SetDiffuseTexture(MEM::Ref<Texture> texture) { _DiffuseTexture = texture; }
	void SetRoughnessTexture(MEM::Ref<Texture> texture) { _RoughnessTexture = texture; }
	void SetNormalTexture(MEM::Ref<Texture> texture) { _NormalTexture = texture; }
private:
	uint32_t _Id = UINT32_MAX;
	VkDescriptorSet _MaterialSet = VK_NULL_HANDLE;
	MEM::Ref<Texture> _DiffuseTexture = nullptr;
	MEM::Ref<Texture> _RoughnessTexture = nullptr;
	MEM::Ref<Texture> _NormalTexture = nullptr;

	friend class Model;
	friend class Commands;
};

class Mesh {
public:
	Mesh(const std::string& name) : _Name(name) { }
	~Mesh() {}
	const std::string& GetName() const { return _Name; }
	void SetIsVisible(bool visible) { _IsVisible = visible; }
	void SetVertexOffset(uint32_t offset) { _VertexOffset = offset; }
	void SetIndexOffset(uint32_t offset)  { _IndexOffset = offset; }
	void SetIndexCount(uint32_t count)    { _IndexCount = count; }
	void SetMaterialId(uint32_t id) { _MaterialId = id; }
	uint32_t GetVertexOffset() const { return _VertexOffset; }
	uint32_t GetIndexOffset()  const { return _IndexOffset; }
	uint32_t GetIndexCount()   const { return _IndexCount; }
	uint32_t GetMaterialId() const { return _MaterialId; }
	bool IsVisible() const { return _IsVisible; }
private:
	std::string _Name;
	uint32_t _VertexOffset;
	uint32_t _IndexOffset;
	uint32_t _IndexCount;
	uint32_t _MaterialId = UINT32_MAX;

	bool _IsVisible = true;
};

class Model
{
public:
	Model() = default;
	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

	void Bind(VkCommandBuffer commandBuffer);
	std::vector<Mesh>& GetMeshes() { return _Meshes; }
	const std::vector<Mesh>& GetMeshes() const { return _Meshes; }
	const std::unordered_map<uint32_t, Material>& GetMaterials() const { return _Materials; }
	Material& GetMaterial(int index) { return _Materials[index]; }

	static MEM::Ref<Model> CreateModelFromFile(const std::string& path, VkDescriptorImageInfo fallbackTextureInfo, DescriptorPool& pool, DescriptorSetLayout& layout);
private:
	void LoadModelFromFile(const std::string& path, VkDescriptorImageInfo fallbackTextureInfo, DescriptorPool& pool, DescriptorSetLayout& layout);
	void CreateVertexBuffer(const std::vector<Vertex>& vertices);
	void CreateIndexBuffer(const std::vector<uint32_t>& indices);
	void ProcessMaterials(const aiScene* scene, const std::string& modelPath, VkDescriptorImageInfo fallbackTextureInfo, DescriptorPool& pool, DescriptorSetLayout& layout);
	MEM::Ref<Texture> LoadMaterialTexture(const std::string& path, std::unordered_map<std::string, MEM::Ref<Texture>>& cache);

	MEM::Scope<Buffer> _VertexBuffer;
	uint32_t _VertexCount;

	bool _HasIndexBuffer = false;
	MEM::Scope<Buffer> _IndexBuffer;
	uint32_t _IndexCount;

	std::vector<Mesh> _Meshes;
	std::vector<Vertex> _Vertices{};
	std::vector<uint32_t> _Indices{};
	std::unordered_map<uint32_t, Material> _Materials;
};