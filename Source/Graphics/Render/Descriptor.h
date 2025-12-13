#pragma once

#include <vulkan/vulkan.h>
#include <unordered_map>
#include <memory>
#include <vector>

class DescriptorSetLayout {
public:
	class Builder {
	public:
		Builder() {}
		Builder& AddBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count = 1);
		MEM::Scope<DescriptorSetLayout> Build() const;
	private:
		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> _Bindings{};
	};
public:
	DescriptorSetLayout(std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
	~DescriptorSetLayout();
	DescriptorSetLayout(const DescriptorSetLayout&) = delete;
	DescriptorSetLayout& operator=(const DescriptorSetLayout&) = delete;

	VkDescriptorSetLayout GetDescriptorSetLayout() const { return _DescriptorSetLayout; }
private:
	VkDescriptorSetLayout _DescriptorSetLayout;
	std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> _Bindings;
	friend class DescriptorWriter;
};

class DescriptorPool {
public:
	class Builder {
	public:
		Builder() {}
		Builder& AddPoolSize(VkDescriptorType descriptorType, uint32_t count);
		Builder& SetPoolFlags(VkDescriptorPoolCreateFlags flags);
		Builder& SetMaxSets(uint32_t count);
		MEM::Scope<DescriptorPool> Build() const;
	private:
		uint32_t _MaxSets = 1000;
		std::vector<VkDescriptorPoolSize> _PoolSizes{};
		VkDescriptorPoolCreateFlags _PoolFlags = 0;
	};
public:
	DescriptorPool(uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes);
	~DescriptorPool();
	DescriptorPool(const DescriptorPool&) = delete;
	DescriptorPool& operator=(const DescriptorPool&) = delete;

	VkDescriptorPool GetDescriptorPool() const { return _DescriptorPool; }

	bool Allocate(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const;
	void Free(std::vector<VkDescriptorSet>& descriptors);
	void Reset();
private:
	VkDescriptorPool _DescriptorPool;
	friend class DescriptorWriter;
};

class DescriptorWriter {
public:
	DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool);
	DescriptorWriter& WriteBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo);
	DescriptorWriter& WriteImage(uint32_t binding, VkDescriptorImageInfo* imageInfo);
	bool Build(VkDescriptorSet& set);
	void Overwrite(VkDescriptorSet& set);
private:
	DescriptorSetLayout& _SetLayout;
	DescriptorPool& _Pool;
	std::vector<VkWriteDescriptorSet> _Writes;
};