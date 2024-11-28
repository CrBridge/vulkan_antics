#include "va_cubemap.hpp"

#include "va_buffer.hpp"

#include <string>
#include <array>
#include <stdexcept>

#ifndef FILE_DIR
#define FILE_DIR "../../../"
#endif

namespace va {
	VaCubemap::VaCubemap(VaDevice& device)
		: vaDevice{ device } {
		createCubemap();
		createImageView();
		createSampler();
		updateDescriptor();
	}

	VaCubemap::~VaCubemap() {
		vkDestroySampler(vaDevice.device(), cubemapSampler, nullptr);
		vkDestroyImageView(vaDevice.device(), cubemapImageView, nullptr);
		vkDestroyImage(vaDevice.device(), cubemapImage, nullptr);
		vkFreeMemory(vaDevice.device(), cubemapImageMemory, nullptr);
	}

	void VaCubemap::createCubemap() {
		int texWidth{}, texHeight{}, texChannels{};

		stbi_set_flip_vertically_on_load(true);
		stbi_uc* skyboxPixels[6];

		std::array<std::string, 6> skyboxPaths = {
			"textures/skybox/skycube-right.png",
			"textures/skybox/skycube-left.png",
			"textures/skybox/skycube-down.png",
			"textures/skybox/skycube-up.png",
			"textures/skybox/skycube-front.png",
			"textures/skybox/skycube-back.png",
		};

		for (int i = 0; i < 6; i++) {
			skyboxPixels[i] = loadImage(FILE_DIR + skyboxPaths[i], &texWidth, &texHeight, &texChannels);
			if (!skyboxPixels[i]) {
				throw std::runtime_error("failed to load texture image");
			}
		}

		VkDeviceSize imageSize = texWidth * texHeight * 4 * 6;
		VkDeviceSize layerSize = imageSize / 6;

		VaBuffer stagingBuffer{
			vaDevice,
			imageSize,
			1,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		stagingBuffer.map();
		for (int i = 0; i < 6; i++) {
			stagingBuffer.writeToBuffer((void*)skyboxPixels[i], layerSize, i * layerSize);
			stbi_image_free(skyboxPixels[i]);
		}

		createImage(texWidth, texHeight);

		vaDevice.transitionImageLayout(
			cubemapImage,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			6
		);
		vaDevice.copyBufferToImage(
			stagingBuffer.getBuffer(),
			cubemapImage,
			static_cast<uint32_t>(texWidth),
			static_cast<uint32_t>(texHeight), 6
		);
		vaDevice.transitionImageLayout(
			cubemapImage,
			VK_FORMAT_R8G8B8A8_SRGB,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			6
		);
	}

	stbi_uc* VaCubemap::loadImage(const std::string& filepath, int* width, int* height, int* channels) {
		return stbi_load(filepath.c_str(), width, height, channels, STBI_rgb_alpha);
	}

	void VaCubemap::createImage(uint32_t width, uint32_t height) {
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 6;
		imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

		vaDevice.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, cubemapImage, cubemapImageMemory);
	}

	void VaCubemap::createImageView() {
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = cubemapImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 6;

		if (vkCreateImageView(vaDevice.device(), &viewInfo, nullptr, &cubemapImageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view");
		}
	}

	void VaCubemap::createSampler() {
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = vaDevice.properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		if (vkCreateSampler(vaDevice.device(), &samplerInfo, nullptr, &cubemapSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler");
		}
	}

	void VaCubemap::updateDescriptor() {
		cubemapDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		cubemapDescriptorInfo.imageView = cubemapImageView;
		cubemapDescriptorInfo.sampler = cubemapSampler;
	}
}