#include "gpch.h"
#include "SwapChain.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <limits>
#include <set>
#include <stdexcept>
#include "Engine/Application.h"

VkSurfaceFormatKHR SwapChain::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
{
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			return availableFormat;
	}
	return availableFormats[0];
}
VkPresentModeKHR SwapChain::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) { 
	for (const auto& availablePresentMode : availablePresentModes) {
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
			return availablePresentMode;
		}
	}
	return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D SwapChain::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	}
	else {
		VkExtent2D actualExtend = _WindowExtent;
		actualExtend.width  = std::max(capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtend.width));
		actualExtend.height = std::max(capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtend.height));
		return actualExtend;
	}
}

SwapChain::SwapChain(VkExtent2D windowExtent) : _WindowExtent(windowExtent)
{
	Init();
}

SwapChain::SwapChain(VkExtent2D windowExtent, SwapChain* previus)
	: _WindowExtent(windowExtent), _OldSwapChain(previus)
{
	Init();
	_OldSwapChain = nullptr;
}

void SwapChain::Init() {
	try {
		CreateSwapChain();
		CreateImageViews();
		CreateRenderPass();
		CreateDepthresources();
		CreateFramebuffers();
		CreateSyncObjects();
	}
	catch (const std::exception& e) {
		LOG_ERROR(e.what());
		return;
	}
}

SwapChain::~SwapChain()
{
	vkDeviceWaitIdle(Application::Get()->GetDevice().LogicalDevice());

	for (auto imageView : _SwapChainImageViews)
		vkDestroyImageView(Application::Get()->GetDevice().LogicalDevice(), imageView, nullptr);
	_SwapChainImageViews.clear();

	for (int i = 0; i < _DepthImages.size(); i++) {
		vkDestroyImageView(Application::Get()->GetDevice().LogicalDevice(), _DepthImageViews[i],    nullptr);
		vkDestroyImage(Application::Get()->GetDevice().LogicalDevice(),	    _DepthImages[i],        nullptr);
		vkFreeMemory(Application::Get()->GetDevice().LogicalDevice(),       _DepthImageMemorys[i],  nullptr);
	}
	_DepthImageViews.clear();
	_DepthImages.clear();
	_DepthImageMemorys.clear();

	for (auto framebuffer : _SwapChainFramebuffers)
		vkDestroyFramebuffer(Application::Get()->GetDevice().LogicalDevice(), framebuffer, nullptr);

	vkDestroyRenderPass(Application::Get()->GetDevice().LogicalDevice(), _RenderPass, nullptr);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(Application::Get()->GetDevice().LogicalDevice(), _ImageAvailableSemaphores[i], nullptr);
		vkDestroyFence(Application::Get()->GetDevice().LogicalDevice(),     _InFlightFences[i], nullptr);
	}

	for (size_t i = 0; i < _RenderFinishedSemaphores.size(); i++) {
		vkDestroySemaphore(Application::Get()->GetDevice().LogicalDevice(), _RenderFinishedSemaphores[i], nullptr);
	}

	if (_SwapChain != nullptr) {
		vkDestroySwapchainKHR(Application::Get()->GetDevice().LogicalDevice(), _SwapChain, nullptr);
		_SwapChain = nullptr;
	}
}

VkFormat SwapChain::FindDepthFormat()
{
	return Application::Get()->GetDevice().FindSupportedFormat({ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkResult SwapChain::AcquireNextImage(uint32_t* imageIndex)
{
	vkWaitForFences(Application::Get()->GetDevice().LogicalDevice(), 1, &_InFlightFences[_CurrentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());
	VkResult result = vkAcquireNextImageKHR(
		Application::Get()->GetDevice().LogicalDevice(),
		_SwapChain,
		std::numeric_limits<uint64_t>::max(),
		_ImageAvailableSemaphores[_CurrentFrame],
		VK_NULL_HANDLE,
		imageIndex);
	return result;
}

VkResult SwapChain::SubmitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex)
{
	if (_ImagesInFlight[*imageIndex] != VK_NULL_HANDLE)
		vkWaitForFences(Application::Get()->GetDevice().LogicalDevice(), 1, &_ImagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
	_ImagesInFlight[*imageIndex] = _InFlightFences[_CurrentFrame];

	VkSubmitInfo submitInfo{};
	VkSemaphore waitSemphores[] = { _ImageAvailableSemaphores[_CurrentFrame] };
	VkSemaphore signalSemaphores[] = { _RenderFinishedSemaphores[*imageIndex] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = buffers;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(Application::Get()->GetDevice().LogicalDevice(), 1, &_InFlightFences[_CurrentFrame]);
	ASSERT(vkQueueSubmit(Application::Get()->GetDevice().GraphicsQueue(), 1, &submitInfo, _InFlightFences[_CurrentFrame]) == VK_SUCCESS);

	VkPresentInfoKHR presentInfo{};
	VkSwapchainKHR swapChains[] = { _SwapChain };
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = imageIndex;

	VkResult result = vkQueuePresentKHR(Application::Get()->GetDevice().PresentQueue(), &presentInfo);
	_CurrentFrame = (_CurrentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	return result;
}

void SwapChain::CreateSwapChain()
{
	SwapChainSupportDetails swapChainSupport = Application::Get()->GetDevice().GetSwapChainSupport();
	VkSurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(swapChainSupport.Formats);
	VkPresentModeKHR presentMode = ChooseSwapPresentMode(swapChainSupport.PresentModes);
	VkExtent2D extent = ChooseSwapExtent(swapChainSupport.Capabilities);
	uint32_t imageCount = swapChainSupport.Capabilities.minImageCount + 1;
	if (swapChainSupport.Capabilities.maxImageCount > 0 &&
		imageCount > swapChainSupport.Capabilities.maxImageCount) {
		imageCount = swapChainSupport.Capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = Application::Get()->GetDevice().Surface();
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = Application::Get()->GetDevice().FindPhysicalQueueFamilies();
	uint32_t queueFamilyIndices[] = { indices.GrapichsFamily, indices.PresentFamily };
	if (indices.GrapichsFamily != indices.PresentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}

	createInfo.preTransform = swapChainSupport.Capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;

	createInfo.oldSwapchain = _OldSwapChain == nullptr ? VK_NULL_HANDLE : _OldSwapChain->_SwapChain;

	if (vkCreateSwapchainKHR(Application::Get()->GetDevice().LogicalDevice (), &createInfo, nullptr, &_SwapChain) != VK_SUCCESS) {
		throw std::runtime_error("failed to create swap chain!");
	}

	vkGetSwapchainImagesKHR(Application::Get()->GetDevice().LogicalDevice(), _SwapChain, &imageCount, nullptr);
	_SwapChainImages.resize(imageCount);
	vkGetSwapchainImagesKHR(Application::Get()->GetDevice().LogicalDevice(), _SwapChain, &imageCount, _SwapChainImages.data());

	_SwapChainImageFormat = surfaceFormat.format;
	_SwapChainExtent = extent;
}

void SwapChain::CreateImageViews()
{
	_SwapChainImageViews.resize(_SwapChainImages.size());
	for (size_t i = 0; i < _SwapChainImages.size(); i++) {
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = _SwapChainImages[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = _SwapChainImageFormat;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;
		if (vkCreateImageView(Application::Get()->GetDevice().LogicalDevice(), &viewInfo, nullptr, &_SwapChainImageViews[i]) !=
			VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}
	}
}

void SwapChain::CreateDepthresources()
{
	VkFormat depthFormat = FindDepthFormat();
	_SwapChainDepthFormat = depthFormat;
	VkExtent2D swapChainExtent = GetSwapChainExtent();

	_DepthImages.resize(GetImageCount());
	_DepthImageMemorys.resize(GetImageCount());
	_DepthImageViews.resize(GetImageCount());

	for (int i = 0; i < _DepthImages.size(); i++) {
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = swapChainExtent.width;
		imageInfo.extent.height = swapChainExtent.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = depthFormat;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.flags = 0;

		Application::Get()->GetDevice().CreateImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, _DepthImages[i], _DepthImageMemorys[i]);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = _DepthImages[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = depthFormat;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(Application::Get()->GetDevice().LogicalDevice(), &viewInfo, nullptr, &_DepthImageViews[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view!");
		}
	}
}

void SwapChain::CreateRenderPass()
{
	VkAttachmentDescription depthAttachment{};
	depthAttachment.format = FindDepthFormat();
	depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment = 1;
	depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentDescription colorAttachment = {};
	colorAttachment.format = GetSwapChainImageFormat();
	colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference colorAttachmentRef = {};
	colorAttachmentRef.attachment = 0;
	colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass = {};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency = {};
	dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	dependency.srcAccessMask = 0;
	dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstSubpass = 0;
	dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	VkRenderPassCreateInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &dependency;

	if (vkCreateRenderPass(Application::Get()->GetDevice().LogicalDevice(), &renderPassInfo, nullptr, &_RenderPass) != VK_SUCCESS) {
		throw std::runtime_error("failed to create render pass!");
	}
}

void SwapChain::CreateFramebuffers()
{
	_SwapChainFramebuffers.resize(GetImageCount());
	for (size_t i = 0; i < GetImageCount(); i++) {
		std::array<VkImageView, 2> attachments = { _SwapChainImageViews[i], _DepthImageViews[i] };
		VkExtent2D swapChainExtent = GetSwapChainExtent();
		VkFramebufferCreateInfo framebufferInfo = {};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = _RenderPass;
		framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments = attachments.data();
		framebufferInfo.width = swapChainExtent.width;
		framebufferInfo.height = swapChainExtent.height;
		framebufferInfo.layers = 1;

		if (vkCreateFramebuffer(Application::Get()->GetDevice().LogicalDevice(), &framebufferInfo, nullptr, &_SwapChainFramebuffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}

void SwapChain::CreateSyncObjects()
{
	_ImageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	_InFlightFences.resize(MAX_FRAMES_IN_FLIGHT);
	_ImagesInFlight.resize(GetImageCount(), VK_NULL_HANDLE);
	_RenderFinishedSemaphores.resize(GetImageCount());

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(Application::Get()->GetDevice().LogicalDevice(), &semaphoreInfo, nullptr, &_ImageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateFence(Application::Get()->GetDevice().LogicalDevice(), &fenceInfo, nullptr, &_InFlightFences[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}

	for (size_t i = 0; i < GetImageCount(); i++) {
		if (vkCreateSemaphore(Application::Get()->GetDevice().LogicalDevice(), &semaphoreInfo, nullptr, &_RenderFinishedSemaphores[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to create render finished semaphore!");
		}
	}
}
