#pragma once
#include "Graphics/Device.h"
#include <string>
#include <vector>

class Pipeline
{
public:
	Pipeline(
		const std::string& vertexShaderPath, 
		const std::string& fragmentShaderPath, 
		const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
		VkRenderPass renderPass
	);
	~Pipeline();
	Pipeline(const Pipeline&) = delete;
	Pipeline& operator=(const Pipeline&) = delete;

	void Bind(VkCommandBuffer commandBuffer);
	VkPipelineLayout GetLayout() { return _PipelineLayout; }
private:
	void CreateLayout(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
	static std::vector<char> ReadFile(const std::string& path);
	void CreateGraphicsPipline(const PipelineConfigInfo& config, VkRenderPass renderPass, const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
	void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
private:
	VkPipeline _GraphicsPipeline = VK_NULL_HANDLE;
	VkPipelineLayout _PipelineLayout = VK_NULL_HANDLE;
	VkShaderModule _VertShaderModule = VK_NULL_HANDLE;
	VkShaderModule _FragShaderModule = VK_NULL_HANDLE;
};

