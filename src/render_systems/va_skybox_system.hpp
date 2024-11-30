#pragma once

#include "../va_pipeline.hpp"
#include "../va_device.hpp"
#include "../va_game_object.hpp"
#include "../va_camera.hpp"
#include "../va_frame_info.hpp"
#include "../va_descriptors.hpp"
#include "../va_cubemap.hpp"

#include <memory>
#include <vector>

namespace va {
	class VaSkyboxSystem {
	public:
		VaSkyboxSystem(VaDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~VaSkyboxSystem();

		VaSkyboxSystem(const VaSkyboxSystem&) = delete;
		VaSkyboxSystem& operator=(const VaSkyboxSystem&) = delete;

		void renderSkybox(FrameInfo& frameInfo);

	private:
		VaDevice& vaDevice;
		std::unique_ptr<VaPipeline> vaPipeline;
		VkPipelineLayout pipelineLayout;
		std::unique_ptr<VaDescriptorSetLayout> objDescriptorSetLayout;

		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);
	};
}