#pragma once

#include "va_camera.hpp"
#include "va_game_object.hpp"

#include <vulkan/vulkan.h>

namespace va {
	struct FrameInfo {
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		VaCamera& camera;
		VkDescriptorSet globalDescriptorSet;
		VaGameObject::Map& gameObjects;
	};
}