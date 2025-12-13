#include "gpch.h"
#include "Renderer.h"
#include "Engine/Application.h"

Renderer::Renderer() {
	RecreateSwapChain();
	CreateFrameContexts();
}

Renderer::~Renderer() {
	DestroyFrameContexts();
}

bool Renderer::IsFrameInProgress() const { return _IsFrameStarted; }
float Renderer::GetAspectRatio() const { return _SwapChain->GetAspectRatio(); }
VkRenderPass Renderer::GetSwapChainRenderPass() const { return _SwapChain->GetRenderPass(); }
VkFormat Renderer::GetSwapChainImageFormat() const { return _SwapChain->GetSwapChainImageFormat(); }
VkCommandBuffer Renderer::GetCurrentCommandBuffer() const {
	ASSERT(_IsFrameStarted, "Cannot get command buffer when frame not in progress");
	return _Frames[_CurrentFrameIndex].commandBuffer;
}

int Renderer::GetFrameIndex() const {
	ASSERT(_IsFrameStarted, "Can't call GetFrameIndex while frame is not in progress");
	return _CurrentFrameIndex;
}

void Renderer::Submit(const std::function<void(VkCommandBuffer&)>& fn)
{
	auto swapchainExtent = _SwapChain->GetSwapChainExtent();
	auto cmd = BeginFrame();
	Commands::BeginRenderPass(cmd);
	Commands::SetViewport(cmd, (float)swapchainExtent.width, (float)swapchainExtent.height);
	fn(cmd);
	Commands::EndRenderPass(cmd);
	EndFrame();
}

VkCommandBuffer Renderer::BeginFrame() {
	ASSERT(!_IsFrameStarted, "Can't call beginFrame while already in progress");
	VkResult result = _SwapChain->AcquireNextImage(&_CurrentImageIndex);
	ASSERT(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR);

	_IsFrameStarted = true;
	
	auto& frame = _Frames[_CurrentFrameIndex];
	auto commandBuffer = GetCurrentCommandBuffer();
	VkCommandBufferBeginInfo beginInfo{ VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	ASSERT(vkResetCommandPool(Application::Get()->GetDevice().LogicalDevice(), frame.commandPool, 0) == VK_SUCCESS, "Failed to reset command pool!");
	ASSERT(vkBeginCommandBuffer(commandBuffer, &beginInfo) == VK_SUCCESS, "Failed to begin recording commands");
	return commandBuffer;
}

void Renderer::EndFrame() {
	ASSERT(_IsFrameStarted, "Can't call endFrame while frame is not in progress");
	auto commandBuffer = GetCurrentCommandBuffer();
	ASSERT(vkEndCommandBuffer(commandBuffer) == VK_SUCCESS, "Failed to end recording for command buffer");
	auto result = _SwapChain->SubmitCommandBuffers(&commandBuffer, &_CurrentImageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || Application::Get()->GetWindow().WasWindowResized()) {
		Application::Get()->GetWindow().ResetWindowresizedFlag();
		RecreateSwapChain();
	} else if(result != VK_SUCCESS) { ASSERT(false && "Failed to present swap chain Image!"); }
	_IsFrameStarted = false;
	_CurrentFrameIndex = (_CurrentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::CreateFrameContexts()
{
	auto& device = Application::Get()->GetDevice();
	for (uint32_t i = 0; i < SwapChain::MAX_FRAMES_IN_FLIGHT; i++) {
		auto& frame = _Frames[i];

		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = Application::Get()->GetDevice().FindPhysicalQueueFamilies().GrapichsFamily;

		ASSERT(vkCreateCommandPool(device.LogicalDevice(), &poolInfo, nullptr, &frame.commandPool) == VK_SUCCESS, "Failed to create command pool!");

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = frame.commandPool;
		allocInfo.commandBufferCount = 1;
		ASSERT(vkAllocateCommandBuffers(device.LogicalDevice(), &allocInfo, &frame.commandBuffer) == VK_SUCCESS, "Failed to create command buffers");
	}
}

void Renderer::DestroyFrameContexts()
{
	auto& device = Application::Get()->GetDevice();
	for (auto& frame : _Frames) {
		vkFreeCommandBuffers(device.LogicalDevice(), frame.commandPool, 1, &frame.commandBuffer);
		vkDestroyCommandPool(device.LogicalDevice(), frame.commandPool, nullptr);
	}
}

void Renderer::RecreateSwapChain() {
	auto extent = Application::Get()->GetWindow().GetExtend();
	while (extent.x == 0 || extent.y == 0) {
		extent = Application::Get()->GetWindow().GetExtend();
		glfwWaitEvents();
	}
	vkDeviceWaitIdle(Application::Get()->GetDevice().LogicalDevice());

	if (_SwapChain == nullptr) {
		_SwapChain = MEM::CreateScope<SwapChain>(VkExtent2D(extent.x, extent.y));
	}
	else {
		MEM::Scope<SwapChain> oldSwapChain = std::move(_SwapChain);
		_SwapChain = MEM::CreateScope<SwapChain>(VkExtent2D(extent.x, extent.y), oldSwapChain.get());
		ASSERT(oldSwapChain->CompareSwapFormats(*_SwapChain.get()) && "Swap chain image(||depth) format has changed!");
	}
}

VkExtent2D Renderer::GetSwapChainExtent() const {
	return _SwapChain->GetSwapChainExtent();
}

VkFramebuffer Renderer::GetSwapChainCurrentFramebuffer() const {
	return _SwapChain->GetFramebuffer(_CurrentImageIndex);
}