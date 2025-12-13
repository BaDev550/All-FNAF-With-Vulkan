#pragma once
#include "Graphics/Device.h"
#include <string>
#include <vector>

class Pipeline
{
public:
	Pipeline(const std::string& vertexShaderPath, const std::string& fragmentShaderPath, const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
	Pipeline(const PipelineConfigInfo& config, const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
	~Pipeline();
	Pipeline(const Pipeline&) = delete;
	Pipeline& operator=(const Pipeline&) = delete;

	void Bind(VkCommandBuffer commandBuffer);
	void CreateLayout(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts);
	VkPipelineLayout GetLayout() const { return _PipelineLayout; }
private:
	static std::vector<char> ReadFile(const std::string& path);
	void CreateGraphicsPipline(const PipelineConfigInfo& config, const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
	void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
private:
	VkPipeline _GraphicsPipeline;
	VkPipelineLayout _PipelineLayout;
	VkShaderModule _VertShaderModule;
	VkShaderModule _FragShaderModule;
};

