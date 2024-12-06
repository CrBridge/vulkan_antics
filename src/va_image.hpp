#pragma once

#include "va_device.hpp"
#include "va_descriptors.hpp"

#include <memory>
#include <vector>

namespace va {
	class VaImage {
	public:
		VaImage(VaDevice& device, const std::string& filepath);
		~VaImage();

		VaImage(const VaImage&) = delete;
		VaImage& operator=(const VaImage&) = delete;

		static std::unique_ptr<VaImage> createImageFromFile(VaDevice& device, const std::string& filepath) {
			return std::make_unique<VaImage>(device, filepath);
		}

		VkDescriptorImageInfo getInfo() const { return imageDescriptorInfo; }

	private:
		VaDevice& vaDevice;
		
		uint32_t mipLevels;

		VkImage textureImage;
		VkDeviceMemory textureImageMemory;
		VkImageView textureImageView = nullptr;
		VkSampler textureSampler = nullptr;
		VkDescriptorImageInfo imageDescriptorInfo;

		void createTextureImage(const std::string& filepath);
		void createImage(
			uint32_t width, 
			uint32_t height, 
			VkFormat format, 
			VkImageTiling tiling, 
			VkImageUsageFlags usage, 
			VkMemoryPropertyFlags properties, 
			VkImage& image, 
			VkDeviceMemory& imageMemory,
			uint32_t mipLevels
		);
		void createTextureImageView();
		void createTextureSampler();
		void updateDescriptor();
	};
}