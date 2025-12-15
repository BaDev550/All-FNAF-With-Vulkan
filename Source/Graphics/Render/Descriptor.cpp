#include "gpch.h"
#include "Descriptor.h"

#include <cassert>
#include <stdexcept>

#include "Engine/Application.h"

DescriptorSetLayout::Builder& DescriptorSetLayout::Builder::AddBinding(uint32_t binding, VkDescriptorType descriptorType, VkShaderStageFlags stageFlags, uint32_t count) {
	assert(_Bindings.count(binding) == 0 && "Binding already in use");
	VkDescriptorSetLayoutBinding layoutBinding{};
	layoutBinding.binding = binding;
	layoutBinding.descriptorType = descriptorType;
	layoutBinding.descriptorCount = count;
	layoutBinding.stageFlags = stageFlags;
	_Bindings[binding] = layoutBinding;
	return *this;
}

MEM::Scope<DescriptorSetLayout> DescriptorSetLayout::Builder::Build() const {
	return MEM::CreateScope<DescriptorSetLayout>(_Bindings);
}

DescriptorSetLayout::DescriptorSetLayout(std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
	: _Bindings(bindings)
{
	std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
	for (auto kv : _Bindings) {
		setLayoutBindings.push_back(kv.second);
	}

	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
	descriptorSetLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	descriptorSetLayoutInfo.bindingCount = static_cast<uint32_t>(setLayoutBindings.size());
	descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

	ASSERT(vkCreateDescriptorSetLayout(
		Application::Get()->GetDevice().LogicalDevice(), 
		&descriptorSetLayoutInfo,
		nullptr, 
		&_DescriptorSetLayout) == VK_SUCCESS && "failed to create descriptor set layout!");
}

DescriptorSetLayout::~DescriptorSetLayout() {
	vkDestroyDescriptorSetLayout(Application::Get()->GetDevice().LogicalDevice(), _DescriptorSetLayout, nullptr);
}

DescriptorPool::Builder& DescriptorPool::Builder::AddPoolSize(VkDescriptorType descriptorType, uint32_t count) {
	_PoolSizes.push_back({ descriptorType, count });
	return *this;
}

DescriptorPool::Builder& DescriptorPool::Builder::SetPoolFlags(VkDescriptorPoolCreateFlags flags) {
	_PoolFlags = flags;
	return *this;
}

DescriptorPool::Builder& DescriptorPool::Builder::SetMaxSets(uint32_t count) {
	_MaxSets = count;
	return *this;
}

MEM::Scope<DescriptorPool> DescriptorPool::Builder::Build() const {
	return MEM::CreateScope<DescriptorPool>(_MaxSets, _PoolFlags, _PoolSizes);
}

DescriptorPool::DescriptorPool(uint32_t maxSets, VkDescriptorPoolCreateFlags poolFlags, const std::vector<VkDescriptorPoolSize>& poolSizes)
{
	_DescriptorPool = VK_NULL_HANDLE;

	VkDescriptorPoolCreateInfo descriptorPoolInfo{};
	descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	descriptorPoolInfo.pPoolSizes = poolSizes.data();
	descriptorPoolInfo.maxSets = maxSets;
	descriptorPoolInfo.flags = poolFlags;
	VkResult result = vkCreateDescriptorPool(
		Application::Get()->GetDevice().LogicalDevice(),
		&descriptorPoolInfo,
		nullptr,
		&_DescriptorPool
	);

	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan Descriptor Pool!");
	}
}

DescriptorPool::~DescriptorPool() {
	vkDestroyDescriptorPool(Application::Get()->GetDevice().LogicalDevice(), _DescriptorPool, nullptr);
}

bool DescriptorPool::Allocate(const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet& descriptor) const
{
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = _DescriptorPool;
	allocInfo.pSetLayouts = &descriptorSetLayout;
	allocInfo.descriptorSetCount = 1;
	return (vkAllocateDescriptorSets(Application::Get()->GetDevice().LogicalDevice(), &allocInfo, &descriptor) == VK_SUCCESS);
}

void DescriptorPool::Free(std::vector<VkDescriptorSet>& descriptors)
{
	vkFreeDescriptorSets(Application::Get()->GetDevice().LogicalDevice(), _DescriptorPool, static_cast<uint32_t>(descriptors.size()), descriptors.data());
}

void DescriptorPool::Reset()
{
	vkResetDescriptorPool(Application::Get()->GetDevice().LogicalDevice(), _DescriptorPool, 0);
}

DescriptorWriter::DescriptorWriter(DescriptorSetLayout& setLayout, DescriptorPool& pool)
	: _SetLayout{ setLayout }, _Pool{ pool } 
{
}

DescriptorWriter& DescriptorWriter::WriteBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo) {
	assert(_SetLayout._Bindings.count(binding) == 1 && "Layout does not contain specified binding");

	auto& bindingDescription = _SetLayout._Bindings[binding];
	assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType = bindingDescription.descriptorType;
	write.dstBinding = binding;
	write.pBufferInfo = bufferInfo;
	write.descriptorCount = 1;

	_Writes.push_back(write);
	return *this;
}

DescriptorWriter& DescriptorWriter::WriteImage(uint32_t binding, VkDescriptorImageInfo* imageInfo) {
	assert(_SetLayout._Bindings.count(binding) == 1 && "Layout does not contain specified binding");

	auto& bindingDescription = _SetLayout._Bindings[binding];
	assert(bindingDescription.descriptorCount == 1 && "Binding single descriptor info, but binding expects multiple");

	VkWriteDescriptorSet write{};
	write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	write.descriptorType = bindingDescription.descriptorType;
	write.dstBinding = binding;
	write.pImageInfo = imageInfo;
	write.descriptorCount = 1;

	_Writes.push_back(write);
	return *this;
}

bool DescriptorWriter::Build(VkDescriptorSet& set) {
	bool success = _Pool.Allocate(_SetLayout.GetDescriptorSetLayout(), set);
	if (!success) {
		return false;
	}
	Overwrite(set);
	return true;
}

void DescriptorWriter::Overwrite(VkDescriptorSet& set) {
	for (auto& write : _Writes) {
		write.dstSet = set;
	}
	vkUpdateDescriptorSets(Application::Get()->GetDevice().LogicalDevice(), _Writes.size(), _Writes.data(), 0, nullptr);
}