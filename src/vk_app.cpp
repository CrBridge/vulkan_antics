#include "vk_app.hpp"

#include "render_systems/va_render_system.hpp"
#include "render_systems/va_billboard_system.hpp"
#include "render_systems/va_skybox_system.hpp"

#include "models_meshes/va_terrain.hpp"

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
        alignas(16) glm::mat4 view{ 1.0f };
        alignas(16) glm::mat4 inverseView{ 1.0f };
        alignas(16) glm::mat4 projection{ 1.0f };
        // When testing terrain rendering, this ambient aint used at all, since for now I'm just defaulting the terrain normals all straight up
        //  so they all get the directional light equally
        alignas(16) glm::vec4 ambientLightColor{ 1.0f, 1.0f, 1.0f, 0.0f };
        alignas(16) glm::vec4 lightColor{ 0.4f, 0.2f, 0.6f, 1.0f };
        alignas(16) glm::vec3 directionalLight{ 1.0f, -1.0f, -2.0f };
    };

	VkApp::VkApp() {
        globalSetLayout = VaDescriptorSetLayout::Builder(vaDevice)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_ALL_GRAPHICS)
            .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //skybox
            .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //obj textures
            .addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) //
            .addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT) // terrain textures
            .build();

        globalPool = VaDescriptorPool::Builder(vaDevice)
            .setMaxSets(VaSwapChain::MAX_FRAMES_IN_FLIGHT + 100)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VaSwapChain::MAX_FRAMES_IN_FLIGHT)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VaSwapChain::MAX_FRAMES_IN_FLIGHT)
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

        defaultTexture = std::make_shared<VaImage>(vaDevice, "textures/Debugempty.png");
        cubemap = std::make_shared<VaCubemap>(vaDevice);

        globalDescriptorSets.resize(VaSwapChain::MAX_FRAMES_IN_FLIGHT);
        for (int i = 0; i < globalDescriptorSets.size(); i++) {
            auto bufferInfo = globalUboBuffers[i]->descriptorInfo();
			auto cubemapInfo = cubemap->getInfo();
            VaDescriptorWriter(*globalSetLayout, *globalPool)
                .writeBuffer(0, &bufferInfo)
                .writeImage(1, &cubemapInfo)
                .build(globalDescriptorSets[i]);
        }
        initTerrain();
	    //loadGameObjects();
	}

	VkApp::~VkApp() {}

	void VkApp::run() {
		VaRenderSystem renderSystem{ vaDevice, vaRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
        //VaBillboardSystem billboardSystem{ vaDevice, vaRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };
		VaSkyboxSystem skyboxSystem{ vaDevice, vaRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout() };

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
            camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 15000.0f);

			if (auto commandBuffer = vaRenderer.beginFrame()) {
                int frameIndex = vaRenderer.getFrameIndex();
                FrameInfo frameInfo{ frameIndex, frameTime, commandBuffer, camera, globalDescriptorSets[frameIndex], gameObjects };

                GlobalUbo ubo{};
                ubo.view = camera.getView();
                ubo.inverseView = camera.getInverseView();
                ubo.projection = camera.getProjection();
                globalUboBuffers[frameIndex]->writeToBuffer(&ubo);
                globalUboBuffers[frameIndex]->flush();

                vaRenderer.beginSwapChainRenderPass(commandBuffer);
                skyboxSystem.renderSkybox(frameInfo);
				renderSystem.renderGameObjects(frameInfo);
                //billboardSystem.renderBillboard(frameInfo);
				vaRenderer.endSwapChainRenderPass(commandBuffer);
				vaRenderer.endFrame();
			}
            // something to do with the command pool is causing best-practice complaints. Gotta look into that
            //vkResetCommandPool(vaDevice.device(), vaDevice.getCommandPool(), 0);
		}
		vkDeviceWaitIdle(vaDevice.device());
	}

	void VkApp::loadGameObjects() {
        std::shared_ptr<VaModel> roomModel = VaModel::createModelFromFile(vaDevice, "models/viking_room.obj", 1.0f);
		std::shared_ptr<VaImage> roomTexture = VaImage::createImageFromFile(vaDevice, "textures/viking_room.png");
        auto room = VaGameObject::createGameObject();
        room.model = roomModel;
		room.texture = roomTexture;
        room.transform.translation = { 0.0f, 0.44f, 0.0f };
        room.transform.scale = 0.5f;
        room.transform.rotation = { glm::radians(90.0f), 0.0f, glm::radians(180.0f) };
        gameObjects.emplace(room.getId(), std::move(room));

        std::shared_ptr<VaModel> vaseModel = VaModel::createModelFromFile(vaDevice, "models/flat_vase.obj", 1.0f);
        auto vase = VaGameObject::createGameObject();
        vase.model = vaseModel;
        vase.transform.translation = { -2.0f, 0.5f, 0.0f };
        vase.transform.scale = 1.0f;
        gameObjects.emplace(vase.getId(), std::move(vase));

        std::shared_ptr<VaModel> floorModel = VaModel::createModelFromFile(vaDevice, "models/quad.obj", 1.0f);
        std::shared_ptr<VaImage> floorTexture = VaImage::createImageFromFile(vaDevice, "textures/terrain/terrain_3.png");
        auto floor = VaGameObject::createGameObject();
        floor.model = floorModel;
        floor.texture = floorTexture;
        floor.transform.translation = { 0.0f, 0.5f, 0.0f };
        floor.transform.scale = 3.0f;
        gameObjects.emplace(floor.getId(), std::move(floor));

        std::shared_ptr<VaModel> crateModel = VaModel::createModelFromFile(vaDevice, "models/cube.obj", 1.0f);
        std::shared_ptr<VaImage> crateTexture = VaImage::createImageFromFile(vaDevice, "textures/crate_diffuse.png");
        auto crate = VaGameObject::createGameObject();
        crate.model = crateModel;
        crate.texture = crateTexture;
        crate.transform.translation = { -0.5f, 0.3f, -2.0f };
        crate.transform.scale = 0.2f;
        crate.transform.rotation = { 0.0f, glm::radians(75.0f), 0.0f};
        gameObjects.emplace(crate.getId(), std::move(crate));
        
        for (auto& [id, gameObject] : gameObjects) {
            VkDescriptorSet descriptorSet;

            auto textureInfo = (gameObject.texture != nullptr) ? gameObject.texture->getInfo() : defaultTexture->getInfo();
            VaDescriptorWriter(*globalSetLayout, *globalPool)
                .writeImage(2, &textureInfo)
                .build(descriptorSet);

            gameObject.descriptorSet = descriptorSet;
        }
	}

    void VkApp::initTerrain() {
		std::shared_ptr<VaModel> terrainModel = VaTerrain::createTerrainFromFile(vaDevice, "textures/terrain/iceland_heightmap.png");
        std::shared_ptr<VaImage> terrain1 = VaImage::createImageFromFile(vaDevice, "textures/terrain/terrain_4.png");
        std::shared_ptr<VaImage> terrain2 = VaImage::createImageFromFile(vaDevice, "textures/terrain/terrain_5.png");
        auto terrain = VaGameObject::createGameObject();
		terrain.model = terrainModel;
		terrain.terrainTexture1 = terrain1;
        terrain.terrainTexture2 = terrain2;
        // I need a solution for this whole scale thing. Right now, you can't scale by individual axis, because of normal
        // calculation shenanigans. But this means scaling the terrain is also gonna scale it vertically, messes with
        // the terrain textures, as they are based on y position. Really, I would wanna only scale it by x and z axis.
        // temp solution for now could be for the createTerrain function to take in a x-scale, y-scale param,
        // and adjust it that way
        // Honestly, uniform scaling here isn't all that bad. The main issue is that the height constraints are hardcoded
        // in the fragment shader. I could pass the values in with a descriptor, but I'm trying to minimize using those
        //terrain.transform.scale = 100.0f;
        gameObjects.emplace(terrain.getId(), std::move(terrain));

        for (auto& [id, gameObject] : gameObjects) {
            VkDescriptorSet descriptorSet;
            VaDescriptorWriter writer(*globalSetLayout, *globalPool);

            VaDescriptorWriter(*globalSetLayout, *globalPool)
                .writeImage(3, &gameObject.terrainTexture1->getInfo())
                .writeImage(4, &gameObject.terrainTexture2->getInfo())
                .build(descriptorSet);

            gameObject.descriptorSet = descriptorSet;
        }
    }
}