#pragma once

#include "va_game_object.hpp"
#include "va_window.hpp"

namespace va {
    class VaController {
    public:
        struct KeyMappings {
            int moveLeft = GLFW_KEY_A;
            int moveRight = GLFW_KEY_D;
            int moveForward = GLFW_KEY_W;
            int moveBackward = GLFW_KEY_S;
            int moveUp = GLFW_KEY_SPACE;
            int moveDown = GLFW_KEY_LEFT_SHIFT;
            int lookLeft = GLFW_KEY_LEFT;
            int lookRight = GLFW_KEY_RIGHT;
            int lookUp = GLFW_KEY_UP;
            int lookDown = GLFW_KEY_DOWN;
        };

        void moveInPlaneXZ(GLFWwindow* window, float dt, VaGameObject& gameObject);
        void mouseControl(GLFWwindow* window, float dt, VaGameObject& gameObject);

        KeyMappings keys{};
        float moveSpeed{ 30.f };
        float lookSpeed{ 1.5f };
        bool firstMouse{ true };
    };
}