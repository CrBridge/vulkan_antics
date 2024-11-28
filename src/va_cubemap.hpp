#pragma once

#include "va_device.hpp"
#include "va_descriptors.hpp"

#include <stb_image.h>

namespace va {
	class VaCubemap {
	public:
		VaCubemap(VaDevice& device);
		~VaCubemap();

		VaCubemap(const VaCubemap&) = delete;
		VaCubemap& operator=(const VaCubemap&) = delete;

		VkDescriptorImageInfo getInfo() const { return cubemapDescriptorInfo; }

	private:
		VaDevice& vaDevice;

		VkImage cubemapImage;
		VkDeviceMemory cubemapImageMemory;
		VkImageView cubemapImageView = nullptr;
		VkSampler cubemapSampler = nullptr;
		VkDescriptorImageInfo cubemapDescriptorInfo;

		void createCubemap();
		stbi_uc* loadImage(const std::string& filepath, int* width, int* height, int* channels);
		void createImage(uint32_t width, uint32_t height);
		void createImageView();
		void createSampler();
		void updateDescriptor();
	};
}