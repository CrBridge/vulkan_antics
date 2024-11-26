#pragma once

#include "va_window.hpp"
#include "va_device.hpp"
#include "va_swap_chain.hpp"

#include <memory>
#include <vector>
#include <cassert>

namespace va {
	class VaRenderer {
	public:
		VaRenderer(VaWindow& window, VaDevice& device);
		~VaRenderer();

		VaRenderer(const VaRenderer&) = delete;
		VaRenderer& operator=(const VaRenderer&) = delete;

		VkRenderPass getSwapChainRenderPass() const {
			return vaSwapChain->getRenderPass();
		}

		float getAspectRatio() const {
			return vaSwapChain->extentAspectRatio();
		}

		bool isFrameInProgress() const {
			return isFrameStarted;
		}

		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "cannot get command buffer when frame not in progress");
			return commandBuffers[currentFrameIndex];
		}

		int getFrameIndex() const {
			assert(isFrameStarted && "cannot get frame index when frame not in progress");
			return currentFrameIndex;
		}

		VkCommandBuffer beginFrame();
		void endFrame();

		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

	private:
		VaWindow& vaWindow;
		VaDevice& vaDevice;
		std::unique_ptr<VaSwapChain> vaSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		int currentFrameIndex = 0;
		bool isFrameStarted = false;

		void createCommandBuffers();
		void freeCommandBuffers();
		void recreateSwapChain();
	};
}