#include "va_camera.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <cassert>
#include <limits>

namespace va {
    void VaCamera::setPerspectiveProjection(float fovy, float aspect, float near, float far) {
        assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
        glm::mat4 proj = glm::perspective(fovy, aspect, near, far);
        proj[2][2] = -proj[2][2];
        proj[2][3] = -proj[2][3];
        projectionMatrix = proj;
    }

    void VaCamera::setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up) {
        viewMatrix = glm::lookAt(position, position + direction, up);
    }

    void VaCamera::setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up) {
        setViewDirection(position, target - position, up);
    }

    void VaCamera::setViewYXZ(glm::vec3 position, glm::vec3 rotation) {
        glm::mat4 rotY = glm::rotate(glm::mat4(1.0f), rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 rotX = glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        glm::mat4 rotZ = glm::rotate(glm::mat4(1.0f), rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

        glm::mat4 rotationMatrix = rotY * rotX * rotZ;

        glm::vec3 forward = glm::vec3(rotationMatrix * glm::vec4(0.0f, 0.0f, -1.0f, 0.0f));
        glm::vec3 up = glm::vec3(rotationMatrix * glm::vec4(0.0f, 1.0f, 0.0f, 0.0f));

        viewMatrix = glm::lookAt(position, position + forward, up);
    }

}