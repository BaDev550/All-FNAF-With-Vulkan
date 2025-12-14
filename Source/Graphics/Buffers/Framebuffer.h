#pragma once
#include <vulkan/vulkan.h>
#include "imgui.h"
#include "backends/imgui_impl_vulkan.h"

class Framebuffer {
public:
    Framebuffer(uint32_t width, uint32_t height);
    ~Framebuffer();

    VkDescriptorSet GetDescriptorSet() const { return _ImGuiDescriptorSet; }

    VkRenderPass GetRenderPass() const { return _RenderPass; }
    VkFramebuffer GetFramebuffer() const { return _Framebuffer; }
    uint32_t GetWidth() const { return _Width; }
    uint32_t GetHeight() const { return _Height; }

    void BeginRenderPass(VkCommandBuffer cmd);
    void EndRenderPass(VkCommandBuffer cmd);
private:
    void CreateResources();
    void CreateRenderPass();
    void CreateSampler();
    void RegisterImGuiTexture();
    void Cleanup();

    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    uint32_t _Width, _Height;

    VkFramebuffer _Framebuffer = VK_NULL_HANDLE;
    VkRenderPass _RenderPass = VK_NULL_HANDLE;
    VkSampler _Sampler = VK_NULL_HANDLE;
    VkDescriptorSet _ImGuiDescriptorSet = VK_NULL_HANDLE;

    VkImage _ColorImage = VK_NULL_HANDLE;
    VkDeviceMemory _ColorMemory = VK_NULL_HANDLE;
    VkImageView _ColorImageView = VK_NULL_HANDLE;

    VkImage _DepthImage = VK_NULL_HANDLE;
    VkDeviceMemory _DepthMemory = VK_NULL_HANDLE;
    VkImageView _DepthImageView = VK_NULL_HANDLE;
};