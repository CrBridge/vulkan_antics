#pragma once

#include "va_model.hpp"
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