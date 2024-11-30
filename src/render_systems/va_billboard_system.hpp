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
	class VaBillboardSystem {
	public:
		VaBillboardSystem(VaDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~VaBillboardSystem();

		VaBillboardSystem(const VaBillboardSystem&) = delete;
		VaBillboardSystem& operator=(const VaBillboardSystem&) = delete;

		void renderBillboard(FrameInfo& frameInfo);

	private:
		VaDevice& vaDevice;
		std::unique_ptr<VaPipeline> vaPipeline;
		VkPipelineLayout pipelineLayout;
		std::unique_ptr<VaDescriptorSetLayout> objDescriptorSetLayout;

		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);
	};
}