#pragma once
#include "Window/Window.h"
#include "Graphics/Device.h"
#include "Graphics/SwapChain.h"

#include "Commands.h"

#include <memory>
#include <functional>

struct FrameContext {
	VkCommandPool commandPool;
	VkCommandBuffer commandBuffer;
};

class Renderer
{
public:
	Renderer();
	~Renderer();
	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;
	VkRenderPass GetSwapChainRenderPass() const;
	VkFormat GetSwapChainImageFormat() const;
	float GetAspectRatio() const;

	void Submit(const std::function<void(VkCommandBuffer&)>& fn);
private:
	void CreateFrameContexts();
	void DestroyFrameContexts();
	void RecreateSwapChain();

	MEM::Scope<SwapChain> _SwapChain;
	std::array<FrameContext, SwapChain::MAX_FRAMES_IN_FLIGHT> _Frames;

	uint32_t _CurrentImageIndex;
	int _CurrentFrameIndex = 0;
	bool _IsFrameStarted = false;

	bool IsFrameInProgress() const;
	VkExtent2D GetSwapChainExtent() const;
	VkFramebuffer GetSwapChainCurrentFramebuffer() const;
	VkCommandBuffer GetCurrentCommandBuffer() const;
	int GetFrameIndex() const;

	VkCommandBuffer BeginFrame();
	void EndFrame();

	friend class Commands;
	friend class SceneRenderer;
};

