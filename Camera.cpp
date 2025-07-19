#include "Camera.hpp"

Camera::Camera() {}
Camera::~Camera() {}
void Camera::update(float deltaTime) {}
void Camera::processMouseMovement(float xoffset, float yoffset) {}
void Camera::processKeyboardInput(int key, float deltaTime) {}
glm::mat4 Camera::getViewMatrix() const { return glm::mat4(1.0f); }
glm::vec3 Camera::getPosition() const { return glm::vec3(0.0f); }
glm::vec3 Camera::getTarget() const { return glm::vec3(0.0f); }
float Camera::getYaw() const { return 0.0f; }
float Camera::getPitch() const { return 0.0f; } 