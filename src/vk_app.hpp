#pragma once

#include "va_window.hpp"
#include "va_device.hpp"
#include "va_game_object.hpp"
#include "va_renderer.hpp"
#include "va_descriptors.hpp"

#include <memory>
#include <vector>

namespace va {
	class VkApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		VkApp();
		~VkApp();

		VkApp(const VkApp&) = delete;
		VkApp& operator=(const VkApp&) = delete;

		void run();

	private:
		VaWindow vaWindow{ WIDTH, HEIGHT, "Vulkan Gaming" };
		VaDevice vaDevice{ vaWindow };
		VaRenderer vaRenderer{ vaWindow, vaDevice };

		std::unique_ptr<VaDescriptorPool> globalPool{};
		std::shared_ptr<VaDescriptorPool> texturePool{};
		VaGameObject::Map gameObjects;

		void loadGameObjects();
	};
}