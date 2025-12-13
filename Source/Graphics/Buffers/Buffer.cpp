#include "gpch.h"
#include "Buffer.h"

#include "Engine/Application.h"

VkDeviceSize Buffer::GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment)
{
	if (minOffsetAlignment > 0)
		return (instanceSize + minOffsetAlignment - 1) & ~(minOffsetAlignment - 1);
	return instanceSize;
}

Buffer::Buffer(VkDeviceSize instanceSize, uint32_t instanceCount, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags mempropFlags, VkDeviceSize minOffsetAlignment)
	: _InstanceSize(instanceSize), _InstanceCount(instanceCount), _UsageFlag(usageFlags), _MemoryPropertyFlags(mempropFlags)
{
	_AlignmentSize = GetAlignment(instanceSize, minOffsetAlignment);
	_BufferSize = _AlignmentSize * instanceCount;
	Application::Get()->GetDevice().CreateBuffer(_BufferSize, usageFlags, mempropFlags, _Buffer, _Memory);
}

Buffer::~Buffer()
{
	Unmap();
	vkDestroyBuffer(Application::Get()->GetDevice().LogicalDevice(), _Buffer, nullptr);
	vkFreeMemory(Application::Get()->GetDevice().LogicalDevice(), _Memory, nullptr);
}

VkResult Buffer::Map(VkDeviceSize size, VkDeviceSize offset)
{
	assert(_Buffer && _Memory && "Called 'Map Buffer' before created");
	return vkMapMemory(Application::Get()->GetDevice().LogicalDevice(), _Memory, offset, size, 0, &_Data);
}

void Buffer::Unmap()
{
	if (_Data) {
		vkUnmapMemory(Application::Get()->GetDevice().LogicalDevice(), _Memory);
		_Data = nullptr;
	}
}

void Buffer::WriteToBuffer(void* data, VkDeviceSize size, VkDeviceSize offset)
{
	assert(_Data && "Cannot copy to unmapped buffer");
	if (size == VK_WHOLE_SIZE) {
		memcpy(_Data, data, _BufferSize);
	}
	else {
		char* memOffset = (char*)_Data;
		memOffset += offset;
		memcpy(memOffset, data, size);
	}
}

VkResult Buffer::Flush(VkDeviceSize size, VkDeviceSize offset)
{
	VkMappedMemoryRange mappedRange{};
	mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange.memory = _Memory;
	mappedRange.offset = offset;
	mappedRange.size = size;
	return vkFlushMappedMemoryRanges(Application::Get()->GetDevice().LogicalDevice(), 1, &mappedRange);
}

VkDescriptorBufferInfo Buffer::DescriptorInfo(VkDeviceSize size, VkDeviceSize offset)
{
	return VkDescriptorBufferInfo{ _Buffer, offset, size };
}

VkResult Buffer::Invalidate(VkDeviceSize size, VkDeviceSize offset)
{
	VkMappedMemoryRange mappedRange{};
	mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
	mappedRange.memory = _Memory;
	mappedRange.offset = offset;
	mappedRange.size = size;
	return vkInvalidateMappedMemoryRanges(Application::Get()->GetDevice().LogicalDevice(), 1, &mappedRange);
}

void Buffer::WriteToIndex(void* data, int index) { WriteToBuffer(data, _InstanceSize, index * _AlignmentSize); }
VkResult Buffer::FlushIndex(int index) { return Flush(_AlignmentSize, index * _AlignmentSize); }
VkDescriptorBufferInfo Buffer::DescriptorInfoForIndex(int index) { return DescriptorInfo(_AlignmentSize, index * _AlignmentSize); }
VkResult Buffer::InvalidateIndex(int index) { return Invalidate(_AlignmentSize, index * _AlignmentSize); }
