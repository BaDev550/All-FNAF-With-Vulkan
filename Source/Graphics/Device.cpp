#include "gpch.h"
#include "Device.h"
#include "Engine/Application.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {
	LOG_ERROR("[Validation Layer] {}", pCallbackData->pMessage);
	return VK_FALSE;
}

VkResult CreateDebugUtilsMessengerEXT(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
	const VkAllocationCallbacks* pAllocator,
	VkDebugUtilsMessengerEXT* pDebugMessenger) {
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
	}
	else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

void DestroyDebugUtilsMessengerEXT(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debugMessenger,
	const VkAllocationCallbacks* pAllocator) {
	auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, pAllocator);
	}
}

void Device::DefaultPipelineConfigInfo(PipelineConfigInfo& configInfo)
{
	configInfo.InputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	configInfo.InputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	configInfo.InputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

	configInfo.ViewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	configInfo.ViewportInfo.viewportCount = 1;
	configInfo.ViewportInfo.scissorCount = 1;
	configInfo.ViewportInfo.pViewports = nullptr;
	configInfo.ViewportInfo.pScissors = nullptr;

	configInfo.RasterizationInfo = {};
	configInfo.RasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	configInfo.RasterizationInfo.depthClampEnable = VK_FALSE;
	configInfo.RasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
	configInfo.RasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
	configInfo.RasterizationInfo.cullMode = VK_CULL_MODE_FRONT_BIT;
	configInfo.RasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE;
	configInfo.RasterizationInfo.lineWidth = 1.0f;
	configInfo.RasterizationInfo.depthBiasEnable = VK_FALSE;
	configInfo.RasterizationInfo.depthBiasConstantFactor = 0.0f;
	configInfo.RasterizationInfo.depthBiasClamp = 0.0f;
	configInfo.RasterizationInfo.depthBiasSlopeFactor = 0.0f;

	configInfo.MultisampleInfo = {};
	configInfo.MultisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	configInfo.MultisampleInfo.sampleShadingEnable = VK_FALSE;
	configInfo.MultisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	configInfo.MultisampleInfo.minSampleShading = 1.0f;
	configInfo.MultisampleInfo.pSampleMask = nullptr;
	configInfo.MultisampleInfo.alphaToCoverageEnable = VK_FALSE;
	configInfo.MultisampleInfo.alphaToOneEnable = VK_FALSE;

	configInfo.ColorBlendAttachment = {};
	configInfo.ColorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	configInfo.ColorBlendAttachment.blendEnable = VK_FALSE;
	configInfo.ColorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
	configInfo.ColorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
	configInfo.ColorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
	configInfo.ColorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	configInfo.ColorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	configInfo.ColorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

	configInfo.ColorBlendInfo = {};
	configInfo.ColorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	configInfo.ColorBlendInfo.logicOpEnable = VK_FALSE;
	configInfo.ColorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
	configInfo.ColorBlendInfo.attachmentCount = 1;
	configInfo.ColorBlendInfo.pAttachments = &configInfo.ColorBlendAttachment;
	configInfo.ColorBlendInfo.blendConstants[0] = 0.0f;
	configInfo.ColorBlendInfo.blendConstants[1] = 0.0f;
	configInfo.ColorBlendInfo.blendConstants[2] = 0.0f;
	configInfo.ColorBlendInfo.blendConstants[3] = 0.0f;

	configInfo.DepthStencilInfo = {};
	configInfo.DepthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	configInfo.DepthStencilInfo.depthTestEnable = VK_TRUE;
	configInfo.DepthStencilInfo.depthWriteEnable = VK_TRUE;
	configInfo.DepthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
	configInfo.DepthStencilInfo.depthBoundsTestEnable = VK_FALSE;
	configInfo.DepthStencilInfo.minDepthBounds = 0.0f;
	configInfo.DepthStencilInfo.maxDepthBounds = 1.0f;
	configInfo.DepthStencilInfo.stencilTestEnable = VK_FALSE;
	configInfo.DepthStencilInfo.front = {};
	configInfo.DepthStencilInfo.back = {};

	configInfo.DynamicStateEnables = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
	configInfo.DynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	configInfo.DynamicStateInfo.pDynamicStates = configInfo.DynamicStateEnables.data();
	configInfo.DynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.DynamicStateEnables.size());
	configInfo.DynamicStateInfo.flags = 0;
}

Device::Device(Window& window, const ApplicationSpecifications& specs) : _Window(window)
{
	try {
		CreateInstance(specs);
		CreateDebugMessenger();
		CreateSurface();
		CreatePickPhysicalDevice();
		CreateLogicalDevice();
		CreateCommandPool();
	}
	catch (const std::exception& e) {
		LOG_ERROR("Failed to create Device: {}", e.what());
		return;
	}
}

Device::~Device()
{
	if (_CommandPool) vkDestroyCommandPool(_LogicalDevice, _CommandPool, nullptr);
	if (_LogicalDevice) vkDestroyDevice(_LogicalDevice, nullptr);
	if (_DebugMessenger) DestroyDebugUtilsMessengerEXT(_Instance, _DebugMessenger, nullptr);
	if (_Surface) vkDestroySurfaceKHR(_Instance, _Surface, nullptr);
	if (_Instance) vkDestroyInstance(_Instance, nullptr);

	LOG_INFO("Vulkan Device and Allocator destroyed successfully.");
}

void Device::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	VkResult result = vkCreateBuffer(_LogicalDevice, &bufferInfo, nullptr, &buffer);
	ASSERT(result == VK_SUCCESS);

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(_LogicalDevice, buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);
	result = vkAllocateMemory(_LogicalDevice, &allocInfo, nullptr, &bufferMemory);
	ASSERT(result == VK_SUCCESS);

	vkBindBufferMemory(_LogicalDevice, buffer, bufferMemory, 0);
}

VkCommandBuffer Device::BeginSingleTimeCommands()
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandPool = _CommandPool;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(_LogicalDevice, &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	return commandBuffer;
}

void Device::EndSingleTimeCommands(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(_GraphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(_GraphicsQueue);
	vkFreeCommandBuffers(_LogicalDevice, _CommandPool, 1, &commandBuffer);
}

void Device::CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
	VkBufferCopy copyRegion{};
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;
	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);
	EndSingleTimeCommands(commandBuffer);
}

void Device::CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height, uint32_t layerCount)
{
	VkCommandBuffer commandBuffer = BeginSingleTimeCommands();
	VkBufferImageCopy region{};
	region.bufferOffset = 0;
	region.bufferRowLength = 0;
	region.bufferImageHeight = 0;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel = 0;
	region.imageSubresource.baseArrayLayer = 0;
	region.imageSubresource.layerCount = layerCount;
	region.imageOffset = { 0, 0, 0 };
	region.imageExtent = { width, height, 1 };
	vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	EndSingleTimeCommands(commandBuffer);
}

void Device::CreateImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory)
{
	VkResult result = vkCreateImage(_LogicalDevice, &imageInfo, nullptr, &image);
	ASSERT(result == VK_SUCCESS);
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(_LogicalDevice, image, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = FindMemoryType(memRequirements.memoryTypeBits, properties);
	result = vkAllocateMemory(_LogicalDevice, &allocInfo, nullptr, &imageMemory);
	ASSERT(result == VK_SUCCESS);
	vkBindImageMemory(_LogicalDevice, image, imageMemory, 0);
}

void Device::CreateInstance(const ApplicationSpecifications& specs)
{
	if (_EnableValidationLayers && !CheckValidationLayerSupport()) {
		throw std::runtime_error("validation layers requested, but not available!");
	}

	VkApplicationInfo appInfo{};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = specs.name.c_str();
	appInfo.applicationVersion = VK_MAKE_VERSION(specs.appVersion, 0, 0);

	appInfo.pEngineName = "No Engine";
	appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion = VK_MAKE_VERSION(1, 0, 0);

	VkInstanceCreateInfo createInfo{};
	auto extensions = GetRequiredExtensions();
	createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo = &appInfo;

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo;
	if (_EnableValidationLayers) {
		createInfo.enabledLayerCount = static_cast<uint32_t>(_ValidationLayers.size());
		createInfo.ppEnabledLayerNames = _ValidationLayers.data();
		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
	}
	else {
		createInfo.enabledLayerCount = 0;
		createInfo.ppEnabledLayerNames = nullptr;
		createInfo.pNext = nullptr;
	}

	createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames = extensions.data();

	VkResult result = vkCreateInstance(&createInfo, nullptr, &_Instance);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("failed to create instance!");
	}
	HasGflwRequiredInstanceExtensions();
	LOG_INFO("[Device] Vulkan Instance created successfully.");
}

void Device::CreateDebugMessenger()
{
	if (!_EnableValidationLayers) return;
	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	PopulateDebugMessengerCreateInfo(createInfo);
	if (CreateDebugUtilsMessengerEXT(_Instance, &createInfo, nullptr, &_DebugMessenger) != VK_SUCCESS) {
		throw std::runtime_error("failed to set up debug messenger!");
	}
}

void Device::CreateSurface()
{
	glfwCreateWindowSurface(_Instance, _Window.GetHandle(), nullptr, &_Surface);
}

void Device::CreatePickPhysicalDevice()
{
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(_Instance, &physicalDeviceCount, NULL);
	if (physicalDeviceCount == 0)
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	LOG_INFO("[Device] Physical Device count: {}", physicalDeviceCount);

	std::vector<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
	vkEnumeratePhysicalDevices(_Instance, &physicalDeviceCount, physicalDevices.data());
	for (const auto& device : physicalDevices) {
		if (IsDeviceSuitable(device)) {
			_PhysicalDevice = device;
			break;
		}
	}
	if (_PhysicalDevice == VK_NULL_HANDLE)
		throw std::runtime_error("failed to find a suitable GPU!");

	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(_PhysicalDevice, &deviceProperties);

	LOG_INFO("[Device] Selected Physical Device Info: ");
	LOG_INFO("         Name: {}", deviceProperties.deviceName);
	LOG_INFO("         API Version: {}", deviceProperties.apiVersion);
}

void Device::CreateLogicalDevice()
{
	QueueFamilyIndices indices = FindQueueFamilies(_PhysicalDevice);
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = { indices.GrapichsFamily, indices.PresentFamily };

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies) {
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = queueFamily;
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.enabledExtensionCount = static_cast<uint32_t>(_DeviceExtensions.size());
	createInfo.ppEnabledExtensionNames = _DeviceExtensions.data();

	VkResult result = vkCreateDevice(_PhysicalDevice, &createInfo, nullptr, &_LogicalDevice);
	if (result != VK_SUCCESS) 
		throw std::runtime_error("failed to create logical device!");

	vkGetDeviceQueue(_LogicalDevice, indices.GrapichsFamily, 0, &_GraphicsQueue);
	vkGetDeviceQueue(_LogicalDevice, indices.PresentFamily, 0, &_PresentQueue);

	LOG_INFO("[Device] Logical Device created successfully.");
}

void Device::CreateCommandPool()
{
	QueueFamilyIndices queueFamilyIndices = FindQueueFamilies(_PhysicalDevice);
	VkCommandPoolCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	createInfo.queueFamilyIndex = queueFamilyIndices.GrapichsFamily;
	createInfo.flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	VkResult result = vkCreateCommandPool(_LogicalDevice, &createInfo, nullptr, &_CommandPool);
	if (result != VK_SUCCESS)
		throw std::runtime_error("failed to create command pool!");
}

bool Device::IsDeviceSuitable(VkPhysicalDevice device)
{
	QueueFamilyIndices indices = FindQueueFamilies(device);
	bool extensionsSupported = CheckDeviceExtensionSupport(device);
	bool swapChainAdequate = false;
	if (extensionsSupported) {
		SwapChainSupportDetails swapChainSupport = QuerySwapChainSupport(device);
		swapChainAdequate = !swapChainSupport.Formats.empty() && !swapChainSupport.PresentModes.empty();
	}
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
	return indices.IsComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}

bool Device::CheckDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
	std::set<std::string> requiredExtensions(_DeviceExtensions.begin(), _DeviceExtensions.end());

	for (const auto& extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}
	return requiredExtensions.empty();
}

bool Device::CheckValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : _ValidationLayers) {
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
			return false;
	}
	return true;
}

void Device::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
	createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	createInfo.pfnUserCallback = DebugCallback;
	createInfo.pUserData = nullptr;
}

void Device::HasGflwRequiredInstanceExtensions()
{
	uint32_t extensionCount = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

	LOG_INFO("[Device] available extensions:");
	std::unordered_set<std::string> available;
	for (const auto& extension : extensions) {
		LOG_INFO("\t{}", extension.extensionName);
		available.insert(extension.extensionName);
	}

	LOG_INFO("[Device] required extensions:");
	auto requiredExtensions = GetRequiredExtensions();
	for (const auto& required : requiredExtensions) {
		LOG_INFO("\t{}", required);
		if (available.find(required) == available.end()) {
			throw std::runtime_error("Missing required glfw extension");
		}
	}
}

QueueFamilyIndices Device::FindQueueFamilies(VkPhysicalDevice device)
{
	QueueFamilyIndices indices;
	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

	for (uint32_t i = 0; i < queueFamilyCount; i++) {
		if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.GrapichsFamily = i;
			indices.GrapichsFamilyHasValue = true;
		}
		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(device, i, _Surface, &presentSupport);
		if (queueFamilies[i].queueCount > 0 && presentSupport) {
			indices.PresentFamily = i;
			indices.PresentFamilyHasValue = true;
		}
		if (indices.IsComplete()) {
			break;
		}
	}
	return indices;
}

SwapChainSupportDetails Device::QuerySwapChainSupport(VkPhysicalDevice device)
{
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, _Surface, &details.Capabilities);

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, _Surface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.Formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, _Surface, &formatCount, details.Formats.data());
	}

	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, _Surface, &presentModeCount, nullptr);

	if (presentModeCount != 0) {
		details.PresentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, _Surface, &presentModeCount, details.PresentModes.data());
	}

	return details;
}

VkFormat Device::FindSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat format : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(_PhysicalDevice, format, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (
			tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}
	ASSERT(false, "Failed to find supported format");
}

uint32_t Device::FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memP;
	vkGetPhysicalDeviceMemoryProperties(_PhysicalDevice, &memP);
	for (uint32_t i = 0; i < memP.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memP.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}
	ASSERT(false, "Failed to find memory type");
	return 0;
}

std::vector<const char*> Device::GetRequiredExtensions()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (_EnableValidationLayers) {
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}
