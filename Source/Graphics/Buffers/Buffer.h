#pragma once
#include <vulkan/vulkan.h>

class Buffer
{
public:
	static VkDeviceSize GetAlignment(VkDeviceSize instanceSize, VkDeviceSize minOffsetAlignment);

	Buffer(
		VkDeviceSize instanceSize, 
		uint32_t instanceCount, 
		VkBufferUsageFlags usageFlags, 
		VkMemoryPropertyFlags mempropFlags, 
		VkDeviceSize minOffsetAlignment = 1
	);
	~Buffer();
	Buffer(const Buffer&) = delete;
	Buffer& operator=(const Buffer&) = delete;

	VkResult Map(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	void Unmap();

	void WriteToBuffer(void* data, VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	VkResult Flush(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	VkDescriptorBufferInfo DescriptorInfo(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);
	VkResult Invalidate(VkDeviceSize size = VK_WHOLE_SIZE, VkDeviceSize offset = 0);

	void WriteToIndex(void* data, int index);
	VkResult FlushIndex(int index);
	VkDescriptorBufferInfo DescriptorInfoForIndex(int index);
	VkResult InvalidateIndex(int index);

	VkBuffer GetBuffer() const { return _Buffer; }
	void* GetData() const { return _Data; }
	uint32_t GetInstanceCount() const { return _InstanceCount; }
	VkDeviceSize GetInstanceSize() const { return _InstanceSize; }
	VkDeviceSize GetBufferSize() const { return _BufferSize; }
	VkMemoryPropertyFlags GetMemoryPropertyFlags() const { return _MemoryPropertyFlags; }
	VkBufferUsageFlags GetUsageFlags() const { return _UsageFlag; }
private:
	void* _Data = nullptr;
	VkBuffer _Buffer = VK_NULL_HANDLE;
	VkDeviceMemory _Memory = VK_NULL_HANDLE;
	VkDeviceSize _BufferSize;
	uint32_t _InstanceCount;
	VkDeviceSize _InstanceSize;
	VkDeviceSize _AlignmentSize;
	VkBufferUsageFlags _UsageFlag;
	VkMemoryPropertyFlags _MemoryPropertyFlags;
};

