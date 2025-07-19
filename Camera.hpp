#pragma once
#include <glm/glm.hpp>

class Camera {
public:
    Camera();
    ~Camera();

    void update(float deltaTime);
    void processMouseMovement(float xoffset, float yoffset);
    void processKeyboardInput(int key, float deltaTime);
    glm::mat4 getViewMatrix() const;
    glm::vec3 getPosition() const;
    glm::vec3 getTarget() const;
    float getYaw() const;
    float getPitch() const;

private:
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 velocity;
    float pitch;
    float yaw;
    float movementSpeed;
    float mouseSensitivity;
}; 