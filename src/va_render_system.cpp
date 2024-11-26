#include "va_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <cassert>

namespace va {
	struct SimplePushConstantData {
		glm::mat4 modelMatrix{ 1.0f };
	};

	VaRenderSystem::VaRenderSystem(VaDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : vaDevice{ device } {
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);
	}

	VaRenderSystem::~VaRenderSystem() {
		vkDestroyPipelineLayout(vaDevice.device(), pipelineLayout, nullptr);
	}

	void VaRenderSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout) {
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

		if (vkCreatePipelineLayout(vaDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create pipeline layout");
		}
	}

	void VaRenderSystem::createPipeline(VkRenderPass renderPass) {
		assert(pipelineLayout != nullptr && "cannot create pipeline before pipeline layout");

		PipelineConfigInfo pipelineConfig{};
		VaPipeline::defaultPipelineConfigInfo(pipelineConfig);
		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		vaPipeline = std::make_unique<VaPipeline>(
			vaDevice,
			"shaders/vert.spv",
			"shaders/frag.spv",
			pipelineConfig
		);
	}

	void VaRenderSystem::renderGameObjects(FrameInfo& frameInfo) {
		vaPipeline->bind(frameInfo.commandBuffer);

		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			0, 1,
			&frameInfo.globalDescriptorSet,
			0, nullptr
		);

		for (auto& kv : frameInfo.gameObjects) {
			auto& obj = kv.second;
			if (obj.model == nullptr) continue;

			SimplePushConstantData push{};
			push.modelMatrix = obj.transform.mat4();

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);

			vkCmdBindDescriptorSets(
				frameInfo.commandBuffer,
				VK_PIPELINE_BIND_POINT_GRAPHICS,
				pipelineLayout,
				1, 1, // Set 1: object descriptor set
				&obj.texture->getDescriptor(),
				0, nullptr);

			obj.model->bind(frameInfo.commandBuffer);
			obj.model->draw(frameInfo.commandBuffer);
		}
	}
}