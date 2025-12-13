#include "ImGuiLayer.h"

#include <imgui.h>
#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_vulkan.h>

#include "Engine/Application.h"

ImGuiLayer::ImGuiLayer()
{
	_DescriptorPool = DescriptorPool::Builder()
		.AddPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 1000)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000)
		.AddPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000)
		.SetPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
		.SetMaxSets(1000)
		.Build();
    auto& device = Application::Get()->GetDevice();
    auto& window = Application::Get()->GetWindow();

    VkRenderPass renderPass = Application::Get()->GetRenderer().GetSwapChainRenderPass();
	VkFormat swapchainImageFormat = Application::Get()->GetRenderer().GetSwapChainImageFormat();

    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(window.GetHandle(), true);

	ImGui_ImplVulkan_PipelineInfo pipelineInfo{};
	pipelineInfo.RenderPass = renderPass;
	pipelineInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
	pipelineInfo.PipelineRenderingCreateInfo = { VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO };
	pipelineInfo.PipelineRenderingCreateInfo.colorAttachmentCount = 1;
	pipelineInfo.PipelineRenderingCreateInfo.pColorAttachmentFormats = &swapchainImageFormat;
	
    ImGui_ImplVulkan_InitInfo initInfo{};
    initInfo.Instance = device.Instance();
    initInfo.PhysicalDevice = device.PhysicalDevice();
    initInfo.Device = device.LogicalDevice();
    initInfo.Queue = device.GraphicsQueue();
    initInfo.DescriptorPool = _DescriptorPool->GetDescriptorPool();
    initInfo.MinImageCount = 3;
    initInfo.ImageCount = 3;
    initInfo.UseDynamicRendering = false;
	initInfo.PipelineInfoMain = pipelineInfo;

    ImGui_ImplVulkan_Init(&initInfo);
	ImGui_ImplVulkan_CreateMainPipeline(&pipelineInfo);
}

ImGuiLayer::~ImGuiLayer() {
	ImGui_ImplVulkan_Shutdown();
}

void ImGuiLayer::BeginFrame() {
	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}

void ImGuiLayer::EndFrame(VkCommandBuffer cmd) {
	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
}