#pragma once

#include "models_meshes/va_model.hpp"
#include "va_image.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>

namespace va {
	struct TransformComponent {
		glm::vec3 translation;
		float scale{ 1.0f };
		glm::vec3 rotation;

		glm::mat4 mat4();
	};

	class VaGameObject {
	public:
		std::shared_ptr<VaModel> model{};
		std::shared_ptr<VaImage> texture{};
		VkDescriptorSet descriptorSet{};
		glm::vec3 color{};
		TransformComponent transform{};

		// With my setup right now textures need to be stored inside the game object
		// I used multiple textures for terrain so here they are.
		// Maybe It could store a vector instead?
		std::shared_ptr<VaImage> terrainTexture1{};
		std::shared_ptr<VaImage> terrainTexture2{};

		using id_t = unsigned int;
		using Map = std::unordered_map<id_t, VaGameObject>;

		static VaGameObject createGameObject() {
			static id_t currentId = 0;
			return VaGameObject{ currentId++ };
		}

		VaGameObject(const VaGameObject&) = delete;
		VaGameObject& operator=(const VaGameObject&) = delete;
		VaGameObject(VaGameObject&&) = default;
		VaGameObject& operator=(VaGameObject&&) = default;

		const id_t getId() {
			return id;
		}

	private:
		id_t id;

		VaGameObject(id_t objId) : id{ objId } {}
	};
}