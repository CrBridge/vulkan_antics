#include "va_window.hpp"

#include <stdexcept>

namespace va {
	VaWindow::VaWindow(int w, int h, std::string name) : width{ w }, height{ h }, windowName{ name } {
		initWindow();
	}

	VaWindow::~VaWindow() {
		glfwDestroyWindow(window);
		glfwTerminate();
	}

	void VaWindow::initWindow() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, frameBufferResizedCallback);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	void VaWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR* surface) {
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS) {
			throw std::runtime_error("failed to create window surface");
		}
	}

	void VaWindow::frameBufferResizedCallback(GLFWwindow* window, int width, int height) {
		auto vaWindow = reinterpret_cast<VaWindow*>(glfwGetWindowUserPointer(window));
		vaWindow->frameBufferResized = true;
		vaWindow->width = width;
		vaWindow->height = height;
	}
}