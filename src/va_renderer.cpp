#include "va_renderer.hpp"

#include <stdexcept>
#include <array>
#include <cassert>

namespace va {
	VaRenderer::VaRenderer(VaWindow& window, VaDevice& device) : vaWindow{ window }, vaDevice{ device } {
		recreateSwapChain();
		createCommandBuffers();
	}

	VaRenderer::~VaRenderer() {
		freeCommandBuffers();
	}

	void VaRenderer::recreateSwapChain() {
		auto extent = vaWindow.getExtent();

		while (extent.width == 0 || extent.height == 0) {
			extent = vaWindow.getExtent();
			glfwWaitEvents();
		}

		vkDeviceWaitIdle(vaDevice.device());

		if (vaSwapChain == nullptr) {
			vaSwapChain = std::make_unique<VaSwapChain>(vaDevice, extent);
		}
		else {
			std::shared_ptr<VaSwapChain> oldSwapChain = std::move(vaSwapChain);
			vaSwapChain = std::make_unique<VaSwapChain>(vaDevice, extent, oldSwapChain);
			if (!oldSwapChain->compareSwapFormats(*vaSwapChain.get())) {
				throw std::runtime_error("swap chain image format has changed");
			}
		}
	}

	void VaRenderer::createCommandBuffers() {
		commandBuffers.resize(VaSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = vaDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(vaDevice.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers");
		}
	}

	void VaRenderer::freeCommandBuffers() {
		vkFreeCommandBuffers(vaDevice.device(), vaDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		commandBuffers.clear();
	}

	VkCommandBuffer VaRenderer::beginFrame() {
		assert(!isFrameStarted && "cannot call begin frame while already in progress");

		auto result = vaSwapChain->acquireNextImage(&currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR) {
			recreateSwapChain();
			return nullptr;
		}

		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
			throw std::runtime_error("failed to acquire swapchain image");
		}

		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer");
		}

		return commandBuffer;
	}

	void VaRenderer::endFrame() {
		assert(isFrameStarted && "cannot call end frame while frame is not in progress");

		auto commandBuffer = getCurrentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer");
		}

		auto result = vaSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);

		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || vaWindow.wasWindowResized()) {
			vaWindow.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS) {
			throw std::runtime_error("failed to present swapchain image");
		}

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % VaSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void VaRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "cannot call begin swap chain render pass if frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "cant begin render pass on command buffer from different frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = vaSwapChain->getRenderPass();
		renderPassInfo.framebuffer = vaSwapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = vaSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(vaSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(vaSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{ {0, 0}, vaSwapChain->getSwapChainExtent() };
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void VaRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) {
		assert(isFrameStarted && "cannot call end swap chain render pass if frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "cant end render pass on command buffer from different frame");

		vkCmdEndRenderPass(commandBuffer);
	}
}