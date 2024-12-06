#include "va_billboard_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <cassert>
#include <memory>

namespace va {

	VaBillboardSystem::VaBillboardSystem(VaDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : vaDevice{ device } {
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	VaBillboardSystem::~VaBillboardSystem() {
		vkDestroyPipelineLayout(vaDevice.device(), pipelineLayout, nullptr);
	}

	void VaBillboardSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
		objDescriptorSetLayout = VaDescriptorSetLayout::Builder(vaDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
			.addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();
		
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ globalSetLayout, objDescriptorSetLayout->getDescriptorSetLayout() };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;

		if (vkCreatePipelineLayout(vaDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout");
		}
	}

	void VaBillboardSystem::createPipeline(VkRenderPass renderPass) {
		assert(pipelineLayout != nullptr && "cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		VaPipeline::defaultPipelineConfigInfo(pipelineConfig);

		auto bindingDescriptions = VaModel::Vertex::getBindingDescriptions();
		bindingDescriptions.clear();
		auto attributeDescriptions = VaModel::Vertex::getAttributeDescriptions();
		attributeDescriptions.clear();
		pipelineConfig.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		pipelineConfig.vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		pipelineConfig.vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
		pipelineConfig.vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		pipelineConfig.vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		vaPipeline = std::make_unique<VaPipeline>(
			vaDevice,
			"shaders/billboard_vert.spv",
			"shaders/billboard_frag.spv",
			pipelineConfig
		);
	}

	void VaBillboardSystem::renderBillboard(FrameInfo& frameInfo) {
		vaPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0, 1,
			&frameInfo.globalDescriptorSet,
			0, nullptr
		);

		vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
	}
}