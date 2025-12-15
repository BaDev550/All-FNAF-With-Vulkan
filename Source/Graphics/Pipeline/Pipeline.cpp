#include "gpch.h"

#include "Pipeline.h"
#include "Engine/Application.h"
#include "ShaderCompiler.h"

Pipeline::Pipeline(
	const std::string& vertexShaderPath, 
	const std::string& fragmentShaderPath,
	const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts,
	VkRenderPass renderPass
) {
	PipelineConfigInfo config{};
	Device::DefaultPipelineConfigInfo(config);

	if (!descriptorSetLayouts.empty()) {
		CreateLayout(descriptorSetLayouts);
	}
	else {
		_PipelineLayout = config.PipelineLayout;
	}
	CreateGraphicsPipline(config, renderPass, vertexShaderPath, fragmentShaderPath);
}

Pipeline::~Pipeline()
{
	vkDestroyShaderModule(Application::Get()->GetDevice().LogicalDevice(), _VertShaderModule, nullptr);
	vkDestroyShaderModule(Application::Get()->GetDevice().LogicalDevice(), _FragShaderModule, nullptr);
	vkDestroyPipelineLayout(Application::Get()->GetDevice().LogicalDevice(), _PipelineLayout, nullptr);
	vkDestroyPipeline(Application::Get()->GetDevice().LogicalDevice(), _GraphicsPipeline, nullptr);
}

void Pipeline::Bind(VkCommandBuffer commandBuffer)
{
	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _GraphicsPipeline);
}

void Pipeline::CreateLayout(const std::vector<VkDescriptorSetLayout>& descriptorSetLayouts)
{
	VkPushConstantRange range{};
	range.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
	range.offset = 0;
	range.size = sizeof(glm::mat4);

	VkPipelineLayoutCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	createInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
	createInfo.pSetLayouts = descriptorSetLayouts.data();
	createInfo.pushConstantRangeCount = 1;
	createInfo.pPushConstantRanges = &range;
	ASSERT(vkCreatePipelineLayout(Application::Get()->GetDevice().LogicalDevice(), &createInfo, nullptr, &_PipelineLayout) == VK_SUCCESS);
}

std::vector<char> Pipeline::ReadFile(const std::string& path) {
	std::ifstream file(path, std::ios::ate | std::ios::binary);
	if (!file.is_open()) {
		std::cerr << "Failed to open file: " << path << std::endl;
		return {};
	}

	size_t fileSize = static_cast<size_t>(file.tellg());
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}

void Pipeline::CreateGraphicsPipline(const PipelineConfigInfo& config, VkRenderPass renderPass, const std::string& vertexShaderPath, const std::string& fragmentShaderPath) {
	ASSERT(_PipelineLayout != VK_NULL_HANDLE && "Cannot create grapichs pipeline:: pipelinelayout is not valid");
	ASSERT(renderPass && "Cannot create grapichs pipeline:: renderPass is not valid");
	try {
		auto vertexShaderCode = pipeline::utils::CompileShaderFileToSpirv(vertexShaderPath);
		auto fragmentShaderCode = pipeline::utils::CompileShaderFileToSpirv(fragmentShaderPath);
		CreateShaderModule(vertexShaderCode, &_VertShaderModule);
		CreateShaderModule(fragmentShaderCode, &_FragShaderModule);
	}
	catch (std::exception& e) {
		std::cout << e.what() << std::endl;
		return;
	}

	VkPipelineShaderStageCreateInfo shaderStages[2];
	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
	shaderStages[0].module = _VertShaderModule;
	shaderStages[0].pName = "main";
	shaderStages[0].flags = 0;
	shaderStages[0].pNext = nullptr;
	shaderStages[0].pSpecializationInfo = nullptr;

	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	shaderStages[1].module = _FragShaderModule;
	shaderStages[1].pName = "main";
	shaderStages[1].flags = 0;
	shaderStages[1].pNext = nullptr;
	shaderStages[1].pSpecializationInfo = nullptr;

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	auto bindingDescriptions = Vertex::GetBindingDescriptions();
	auto attributeDescriptions = Vertex::GetAttributeDescriptions();
	vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
	vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
	vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount = 2;
	pipelineInfo.pStages = shaderStages;
	pipelineInfo.pVertexInputState =	&vertexInputInfo;
	pipelineInfo.pInputAssemblyState =  &config.InputAssemblyInfo;
	pipelineInfo.pViewportState =		&config.ViewportInfo;
	pipelineInfo.pRasterizationState =  &config.RasterizationInfo;
	pipelineInfo.pMultisampleState =	&config.MultisampleInfo;
	pipelineInfo.pColorBlendState =		&config.ColorBlendInfo;
	pipelineInfo.pDepthStencilState =	&config.DepthStencilInfo;
	pipelineInfo.pDynamicState =        &config.DynamicStateInfo;

	pipelineInfo.layout =	  _PipelineLayout;
	pipelineInfo.renderPass = renderPass;
	pipelineInfo.subpass =	  config.subpass;

	pipelineInfo.basePipelineIndex = -1;
	pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
	VkResult result = vkCreateGraphicsPipelines(Application::Get()->GetDevice().LogicalDevice(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &_GraphicsPipeline);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan pipeline!");
	}
}

void Pipeline::CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
	VkResult result = vkCreateShaderModule(Application::Get()->GetDevice().LogicalDevice(), &createInfo, nullptr, shaderModule);
	if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to create Vulkan shader module!");
	}
}
