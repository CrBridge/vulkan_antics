#pragma once

#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>

#include <string>

namespace va {
	class VaWindow {
	public:
		VaWindow(int w, int h, std::string name);
		~VaWindow();

		VaWindow(const VaWindow&) = delete;
		VaWindow& operator=(const VaWindow&) = delete;

		bool shouldClose() {
			return glfwWindowShouldClose(window);
		};

		VkExtent2D getExtent() {
			return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
		}

		bool wasWindowResized() {
			return frameBufferResized;
		}

		void resetWindowResizedFlag() {
			frameBufferResized = false;
		}

		GLFWwindow* getGLFWwindow() const {
			return window;
		}

		void createWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

	private:
		GLFWwindow* window;
		std::string windowName;
		int width;
		int height;
		bool frameBufferResized = false;

		void initWindow();
		static void frameBufferResizedCallback(GLFWwindow* window, int width, int height);
	};
}