#include "va_controller.hpp"

#include <limits>

namespace va {

    void VaController::moveInPlaneXZ(
        GLFWwindow* window, float dt, VaGameObject& gameObject) {
        float yaw = gameObject.transform.rotation.y;
        const glm::vec3 forwardDir{ sin(yaw), 0.f, cos(yaw) };
        const glm::vec3 rightDir{ forwardDir.z, 0.f, -forwardDir.x };
        const glm::vec3 upDir{ 0.f, -1.f, 0.f };

        glm::vec3 moveDir{ 0.f };
        if (glfwGetKey(window, keys.moveForward) == GLFW_PRESS) moveDir += forwardDir;
        if (glfwGetKey(window, keys.moveBackward) == GLFW_PRESS) moveDir -= forwardDir;
        if (glfwGetKey(window, keys.moveRight) == GLFW_PRESS) moveDir += rightDir;
        if (glfwGetKey(window, keys.moveLeft) == GLFW_PRESS) moveDir -= rightDir;
        if (glfwGetKey(window, keys.moveUp) == GLFW_PRESS) moveDir += upDir;
        if (glfwGetKey(window, keys.moveDown) == GLFW_PRESS) moveDir -= upDir;

        if (glm::dot(moveDir, moveDir) > std::numeric_limits<float>::epsilon()) {
            gameObject.transform.translation += moveSpeed * dt * glm::normalize(moveDir);
        }
    }

    void VaController::mouseControl(GLFWwindow* window, float dt, VaGameObject& gameObject) {
        static double lastX = 0.0, lastY = 0.0;
        static bool firstMouse = true;

        double currentX, currentY;
        glfwGetCursorPos(window, &currentX, &currentY);

        if (firstMouse) {
            lastX = currentX;
            lastY = currentY;
            firstMouse = false;
        }

        float deltaX = static_cast<float>(currentX - lastX);
        float deltaY = static_cast<float>(currentY - lastY);

        lastX = currentX;
        lastY = currentY;

        float sensitivity = 2.5f;
        deltaX *= sensitivity;
        deltaY *= sensitivity;

        gameObject.transform.rotation.y += deltaX * dt;
        gameObject.transform.rotation.x -= deltaY * dt;

        gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
        gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());
    }
}