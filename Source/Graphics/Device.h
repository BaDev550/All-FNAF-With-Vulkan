#pragma once
#include <vulkan/vulkan.h>

#include "Window/Window.h"
#include "Memory.h"

#include <set>
#include <unordered_set>
#include <vector>
#include <assert.h>

struct ApplicationSpecifications;
struct PipelineConfigInfo {
	PipelineConfigInfo() = default;
	PipelineConfigInfo(const PipelineConfigInfo&) = delete;
	PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;

	VkPipelineViewportStateCreateInfo ViewportInfo{};
	VkPipelineRasterizationStateCreateInfo RasterizationInfo{};
	VkPipelineInputAssemblyStateCreateInfo InputAssemblyInfo{};
	VkPipelineMultisampleStateCreateInfo MultisampleInfo{};
	VkPipelineColorBlendAttachmentState ColorBlendAttachment{};
	VkPipelineColorBlendStateCreateInfo ColorBlendInfo{};
	VkPipelineDepthStencilStateCreateInfo DepthStencilInfo{};

	std::vector<VkDynamicState> DynamicStateEnables;
	VkPipelineDynamicStateCreateInfo DynamicStateInfo;

	VkPipelineLayout PipelineLayout = nullptr;
	VkRenderPass RenderPass = nullptr;
	uint32_t subpass = 0;
};

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR Capabilities;
	std::vector<VkSurfaceFormatKHR> Formats;
	std::vector<VkPresentModeKHR> PresentModes;
};

struct QueueFamilyIndices {
	uint32_t GrapichsFamily;
	uint32_t PresentFamily;
	bool GrapichsFamilyHasValue = false;
	bool PresentFamilyHasValue = false;
	bool IsComplete() { return GrapichsFamilyHasValue && PresentFamilyHasValue; }
};

class Device
{
public:
	static void DefaultPipelineConfigInfo(PipelineConfigInfo& configInfo);

	Device(Window& window, const ApplicationSpecifications& specs);
	~Device();

	SwapChainSupportDetails GetSwapChainSupport() { return QuerySwapChainSupport(_PhysicalDevice); }
	QueueFamilyIndices FindPhysicalQueueFamilies() { return FindQueueFamilies(_PhysicalDevice); }
	VkFormat FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
	uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	Device(const Device&) = delete;
	Device& operator=(const Device&) = delete;
	Device(Device&&) = delete;
	Device& operator=(Device&&) = delete;

	void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
	VkCommandBuffer BeginSingleTimeCommands();
	void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
	void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
	void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount);
	void CreateImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);

	VkInstance Instance() const { return _Instance; }
	VkCommandPool CommandPool() const { return _CommandPool; }
	VkSurfaceKHR Surface() const { return _Surface; }
	VkDevice LogicalDevice() const { return _LogicalDevice; }
	VkPhysicalDevice PhysicalDevice() const { return _PhysicalDevice; }
	VkQueue GraphicsQueue() const { return _GraphicsQueue; }
	VkQueue PresentQueue() const { return _PresentQueue; }
	VkPhysicalDeviceProperties GetDeviceProperties() const { return _PhysicalDeviceProperties; }
private:
	void CreateInstance(const ApplicationSpecifications& specs);
	void CreateDebugMessenger();
	void CreateSurface();
	void CreatePickPhysicalDevice();
	void CreateLogicalDevice();
	void CreateCommandPool();
	bool IsDeviceSuitable(VkPhysicalDevice device);
	bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
	bool CheckValidationLayerSupport();
	void PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
	void HasGflwRequiredInstanceExtensions();
	QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
	SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
	std::vector<const char*> GetRequiredExtensions();
private:
	const bool _EnableValidationLayers = true;
	Window& _Window;
	VkInstance _Instance = VK_NULL_HANDLE;
	VkSurfaceKHR _Surface = VK_NULL_HANDLE;
	VkPhysicalDevice _PhysicalDevice = VK_NULL_HANDLE;
	VkPhysicalDeviceProperties _PhysicalDeviceProperties;
	VkDevice _LogicalDevice = VK_NULL_HANDLE;
	VkCommandPool _CommandPool = VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT _DebugMessenger;

	VkQueue _GraphicsQueue = VK_NULL_HANDLE;
	VkQueue _PresentQueue = VK_NULL_HANDLE;

	const std::vector<const char*> _ValidationLayers = { "VK_LAYER_KHRONOS_validation" };
	const std::vector<const char*> _DeviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	friend class SwapChain;
};

