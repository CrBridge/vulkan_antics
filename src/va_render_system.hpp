#pragma once

#include "va_pipeline.hpp"
#include "va_device.hpp"
#include "va_game_object.hpp"
#include "va_camera.hpp"
#include "va_frame_info.hpp"
#include "va_descriptors.hpp"

#include <memory>
#include <vector>

namespace va {
	class VaRenderSystem {
	public:
		VaRenderSystem(VaDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~VaRenderSystem();

		VaRenderSystem(const VaRenderSystem&) = delete;
		VaRenderSystem& operator=(const VaRenderSystem&) = delete;

		void renderGameObjects(FrameInfo& frameInfo);

	private:
		VaDevice& vaDevice;
		std::unique_ptr<VaPipeline> vaPipeline;
		VkPipelineLayout pipelineLayout;

		void createDescriptorSetLayout();
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);
	};
}