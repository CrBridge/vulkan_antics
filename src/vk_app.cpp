#include "vk_app.hpp"

#include "va_render_system.hpp"
#include "va_camera.hpp"
#include "va_controller.hpp"
#include "va_buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stdexcept>
#include <array>
#include <cassert>
#include <chrono>

#include <iostream>

namespace va {
    struct GlobalUbo {
        alignas(16) glm::mat4 projectionView{ 1.0f };
        alignas(16) glm::vec4 ambientLightColor{ 1.0f, 1.0f, 1.0f, 0.02f };
        alignas(16) glm::vec3 lightPosition{ -1.0f };
        alignas(16) glm::vec4 lightColor{ 1.0f };
    };

	VkApp::VkApp() {
        globalSetLayout = VaDescriptorSetLayout::Builder(vaDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

        globalPool = VaDescriptorPool::Builder(vaDevice)
            .setMaxSets(VaSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VaSwapChain::MAX_FRAMES_IN_FLIGHT)
            .build();
        texturePool = VaDescriptorPool::Builder(vaDevice)
            .setMaxSets(VaSwapChain::MAX_FRAMES_IN_FLIGHT + 100)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VaSwapChain::MAX_FRAMES_IN_FLIGHT + 100)
            .build();

        globalUboBuffers.resize(VaSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalUboBuffers.size(); i++) {
            globalUboBuffers[i] = std::make_unique<VaBuffer>(
                vaDevice,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
            );
            globalUboBuffers[i]->map();
        }

        defaultTexture = std::make_shared<VaImage>(vaDevice, texturePool, "textures/Debugempty.png");

        globalDescriptorSets.resize(VaSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = globalUboBuffers[i]->descriptorInfo();
            auto imageInfo = defaultTexture->getInfo();
            VaDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, &imageInfo)
                .build(globalDescriptorSets[i]);
        }
	    loadGameObjects();
	}

	VkApp::~VkApp() {}

	void VkApp::run() {
		VaRenderSystem renderSystem{ vaDevice, vaRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
        VaCamera camera{};

        auto viewerObject = VaGameObject::createGameObject();
        viewerObject.transform.translation = { -1.5f, 0.0f, -2.5f };
        VaController cameraController{};

        auto currentTime = std::chrono::high_resolution_clock::now();

		while (!vaWindow.shouldClose()) {
			glfwPollEvents();

            if (glfwGetKey(vaWindow.getGLFWwindow(), GLFW_KEY_ESCAPE) == GLFW_PRESS) {
                glfwSetWindowShouldClose(vaWindow.getGLFWwindow(), GLFW_TRUE);
            }

            auto newTime = std::chrono::high_resolution_clock::now();
            float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
            currentTime = newTime;

            cameraController.moveInPlaneXZ(vaWindow.getGLFWwindow(), frameTime, viewerObject);
            cameraController.mouseControl(vaWindow.getGLFWwindow(), frameTime, viewerObject);
            camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

            float aspect = vaRenderer.getAspectRatio();
            camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 100.0f);

			if (auto commandBuffer = vaRenderer.beginFrame()) {
                int frameIndex = vaRenderer.getFrameIndex();
                FrameInfo frameInfo{ frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex], gameObjects };

                GlobalUbo ubo{};
                ubo.projectionView = camera.getProjection() * camera.getView();
                globalUboBuffers[frameIndex]->writeToBuffer(&ubo);
                globalUboBuffers[frameIndex]->flush();

				vaRenderer.beginSwapChainRenderPass(commandBuffer);
				renderSystem.renderGameObjects(frameInfo);
				vaRenderer.endSwapChainRenderPass(commandBuffer);
				vaRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(vaDevice.device());
	}

	void VkApp::loadGameObjects() {
        std::shared_ptr<VaModel> roomModel = VaModel::createModelFromFile(vaDevice, "models/viking_room.obj");
		std::shared_ptr<VaImage> roomTexture = VaImage::createImageFromFile(vaDevice, texturePool, "textures/viking_room.png");
        auto room = VaGameObject::createGameObject();
        room.model = roomModel;
		room.texture = roomTexture;
        room.transform.translation = { 0.0f, 0.44f, 0.0f };
        room.transform.scale = 0.5f;
        room.transform.rotation = { glm::radians(90.0f), 0.0f, glm::radians(180.0f) };
        gameObjects.emplace(room.getId(), std::move(room));

        std::shared_ptr<VaModel> vaseModel = VaModel::createModelFromFile(vaDevice, "models/flat_vase.obj");
        auto vase = VaGameObject::createGameObject();
        vase.model = vaseModel;
        vase.transform.translation = { -2.0f, 0.5f, 0.0f };
        vase.transform.scale = 1.0f;
        gameObjects.emplace(vase.getId(), std::move(vase));

        std::shared_ptr<VaModel> floorModel = VaModel::createModelFromFile(vaDevice, "models/quad.obj");
        std::shared_ptr<VaImage> floorTexture = VaImage::createImageFromFile(vaDevice, texturePool, "textures/statue.jpg");
        auto floor = VaGameObject::createGameObject();
        floor.model = floorModel;
        floor.texture = floorTexture;
        floor.transform.translation = { 0.0f, 0.5f, 0.0f };
        floor.transform.scale = 3.0f;
        gameObjects.emplace(floor.getId(), std::move(floor));

        for (auto& [id, gameObject] : gameObjects) {
            VkDescriptorSet descriptorSet;

            auto textureInfo = (gameObject.texture != nullptr) ? gameObject.texture->getInfo() : defaultTexture->getInfo();

            VaDescriptorWriter(*globalSetLayout, *texturePool)
                .writeBuffer(0, &globalUboBuffers[0]->descriptorInfo())
                .writeImage(1, &textureInfo)
                .build(descriptorSet);

            gameObject.descriptorSet = descriptorSet;
        }
	}
}