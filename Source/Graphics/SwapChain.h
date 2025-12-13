#pragma once

#include <vulkan/vulkan.h>
#include "Device.h"

#include <string>
#include <vector>
#include <array>

class SwapChain
{
public:
	static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
	SwapChain(VkExtent2D windowExtent);
	SwapChain(VkExtent2D windowExtent, SwapChain* previus);
	~SwapChain();

	SwapChain(const SwapChain&) = delete;
	SwapChain& operator=(const SwapChain&) = delete;

	VkFramebuffer GetFramebuffer(int index) const { return _SwapChainFramebuffers[index]; }
	VkRenderPass GetRenderPass() const { return _RenderPass; }
	VkImageView GetImageView(int index) const { return _SwapChainImageViews[index]; }
	VkFormat GetSwapChainImageFormat() const { return _SwapChainImageFormat; }
	VkExtent2D GetSwapChainExtent() const { return _SwapChainExtent; }
	size_t GetImageCount() const { return _SwapChainImages.size(); }
	uint32_t GetWidth() { return _SwapChainExtent.width; }
	uint32_t GetHeight() { return _SwapChainExtent.height; }
	float GetAspectRatio() { return (float)_SwapChainExtent.width / (float)_SwapChainExtent.height; }
	bool CompareSwapFormats(const SwapChain& swapChain) const { return swapChain._SwapChainDepthFormat == _SwapChainDepthFormat && swapChain._SwapChainImageFormat == _SwapChainImageFormat; }

	VkFormat FindDepthFormat();
	VkResult AcquireNextImage(uint32_t* imageIndex);
	VkResult SubmitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);
private:
	void Init();
	void CreateSwapChain();
	void CreateImageViews();
	void CreateDepthresources();
	void CreateRenderPass();
	void CreateFramebuffers();
	void CreateSyncObjects();
private:
	VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	std::vector<VkFramebuffer> _SwapChainFramebuffers;
	VkFormat _SwapChainImageFormat;
	VkFormat _SwapChainDepthFormat;
	VkExtent2D _SwapChainExtent;
	VkExtent2D _WindowExtent;
	VkRenderPass _RenderPass = VK_NULL_HANDLE;

	std::vector<VkImage> _DepthImages;
	std::vector<VkDeviceMemory> _DepthImageMemorys;
	std::vector<VkImageView> _DepthImageViews;

	std::vector<VkImage> _SwapChainImages;
	std::vector<VkImageView> _SwapChainImageViews;

	VkSwapchainKHR _SwapChain = VK_NULL_HANDLE;
	SwapChain* _OldSwapChain = nullptr;

	std::vector<VkSemaphore> _ImageAvailableSemaphores;
	std::vector<VkSemaphore> _RenderFinishedSemaphores;
	std::vector<VkFence> _InFlightFences;
	std::vector<VkFence> _ImagesInFlight;
	size_t _CurrentFrame = 0;
};

