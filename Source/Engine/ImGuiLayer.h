#pragma once
#include "Graphics/Render/Descriptor.h"

class ImGuiLayer {
public:
	ImGuiLayer();
	~ImGuiLayer();

	void BeginFrame();
	void EndFrame(VkCommandBuffer cmd);
private:
	MEM::Scope<DescriptorPool> _DescriptorPool;
};