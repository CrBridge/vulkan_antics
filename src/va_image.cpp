#include "va_image.hpp"

#include "va_buffer.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <stdexcept>

namespace va {
	VaImage::VaImage(VaDevice& device, std::shared_ptr<VaDescriptorPool> pool, const std::string& filepath) 
		: vaDevice{ device }, descriptorPool{ pool } {
		createTextureImage(filepath);
		createTextureImageView();
		createTextureSampler();
		updateDescriptor();
		createDescriptorSetLayout();
		createDescriptorSet();
	}

	VaImage::~VaImage() {
		vkDestroySampler(vaDevice.device(), textureSampler, nullptr);
		vkDestroyImageView(vaDevice.device(), textureImageView, nullptr);
		vkDestroyImage(vaDevice.device(), textureImage, nullptr);
		vkFreeMemory(vaDevice.device(), textureImageMemory, nullptr);
	}

	void VaImage::createTextureImage(const std::string& filepath) {
		int texWidth, texHeight, texChannels;
		stbi_set_flip_vertically_on_load(true);
		stbi_uc* pixels = stbi_load(filepath.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		VkDeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels) {
			throw std::runtime_error("failed to load texture image!");
		}

		VaBuffer stagingBuffer{
			vaDevice,
			imageSize,
			1,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer((void*)pixels);

		stbi_image_free(pixels);

		createImage(
			texWidth, 
			texHeight, 
			VK_FORMAT_R8G8B8A8_SRGB, 
			VK_IMAGE_TILING_OPTIMAL, 
			VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, 
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			textureImage, 
			textureImageMemory
		);

		vaDevice.transitionImageLayout(
			textureImage, 
			VK_FORMAT_R8G8B8A8_SRGB, 
			VK_IMAGE_LAYOUT_UNDEFINED, 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
		);
		vaDevice.copyBufferToImage(
			stagingBuffer.getBuffer(),
			textureImage, 
			static_cast<uint32_t>(texWidth), 
			static_cast<uint32_t>(texHeight), 1
		);
		vaDevice.transitionImageLayout(
			textureImage, 
			VK_FORMAT_R8G8B8A8_SRGB, 
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		);
	}

	void VaImage::createImage(
		uint32_t width,
		uint32_t height,
		VkFormat format,
		VkImageTiling tiling,
		VkImageUsageFlags usage,
		VkMemoryPropertyFlags properties,
		VkImage& image,
		VkDeviceMemory& imageMemory
	) {
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.flags = 0;

		vaDevice.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, image, imageMemory);
	}

	void VaImage::createTextureImageView() {
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = textureImage;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(vaDevice.device(), &viewInfo, nullptr, &textureImageView) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture image view");
		}
	}

	void VaImage::createTextureSampler() {
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = VK_FILTER_LINEAR;
		samplerInfo.minFilter = VK_FILTER_LINEAR;
		samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
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

		if (vkCreateSampler(vaDevice.device(), &samplerInfo, nullptr, &textureSampler) != VK_SUCCESS) {
			throw std::runtime_error("failed to create texture sampler");
		}
	}

	void VaImage::updateDescriptor() {
		imageDescriptorInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageDescriptorInfo.imageView = textureImageView;
		imageDescriptorInfo.sampler = textureSampler;
	}

	void VaImage::createDescriptorSetLayout() {
		VkDescriptorSetLayoutBinding binding{};
		binding.binding = 1;
		binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		binding.descriptorCount = 1;
		binding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		binding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo layoutCreateInfo{};
		layoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutCreateInfo.bindingCount = 1;
		layoutCreateInfo.pBindings = &binding;

		if (vkCreateDescriptorSetLayout(vaDevice.device(), &layoutCreateInfo, nullptr, &descriptorSetLayout) != VK_SUCCESS) {
			throw std::runtime_error("failed to create descriptor set layout for image!");
		}
	}

	void VaImage::createDescriptorSet() {
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool->getDescriptorPool();  // You should already have a descriptor pool
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSetLayout;

		if (vkAllocateDescriptorSets(vaDevice.device(), &allocInfo, &descriptorSet) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate descriptor set for image!");
		}

		// Prepare the image info for the descriptor set
		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageView = textureImageView;  // The image view of the texture
		imageInfo.sampler = textureSampler;      // The sampler associated with the texture
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;  // The layout for reading the texture in the shader

		// Create the descriptor write
		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = 0;  // The binding location in the shader (must match)
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;

		// Update the descriptor set with the image info
		vkUpdateDescriptorSets(vaDevice.device(), 1, &descriptorWrite, 0, nullptr);
	}
}
