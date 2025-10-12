#pragma once
#include "VkZero/uniform_buffer.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <vector>


struct TransformUBO {
  glm::mat4 view;
  glm::mat4 proj;
};

namespace VkZero
{
  class Window;
}
class Camera {
public:
  Camera(std::unique_ptr<VkZero::Window> &window);
  ~Camera();

  void update(std::unique_ptr<VkZero::Window> &Window,
              std::unique_ptr<class VoxelWorld> &voxelWorld, int currentFrame);
  void processMouseMovement(float xoffset, float yoffset);
  void processKeyboardInput(int key, float deltaTime);
  glm::mat4 getViewMatrix() const;
  glm::vec3 getPosition() const;
  glm::vec3 getTarget() const;
  float getYaw() const;
  float getPitch() const;

  void onMouseMove(double xPos, double yPos);
  glm::ivec3 rayCast(std::unique_ptr<class VoxelWorld> &voxelWorld,
                     glm::vec3 rayOrigin, glm::vec3 rayDir, bool &something_hit,
                     float maxDistance);
  bool rayIntersectsCube(glm::vec3 corner, float size, glm::vec3 rayOrigin,
                         glm::vec3 rayDir, float &tMin, float &tMax);

  VkZero::UniformBuffer<TransformUBO> uniformBuffer;
  TransformUBO ubo{};

  glm::vec3 cameraTargetPoint = glm::vec3(1.0f, 0.0f, 0.0f);
  glm::vec3 cameraPosition =
      glm::vec3(-512, -512, -512); // glm::vec3(-512.0f, -512.0f, -512.0f);
  glm::ivec3 chunkPosition = glm::ivec3(0, 0, 0);

  glm::vec3 cameraVelocity = glm::vec3(0.0f, 0.0f, 0.0f);

  float pitch = 0.0f; // Rotation around the X-axis (up/down)
  float yaw = -90.0f; // Rotation around the Y-axis (left/right)
  float lastMouseX = 0.0f;
  float lastMouseY = 0.0f;

  float mouseX = 0.0f, mouseY = 0.0f;

  float movementSpeed = 7.0f;
  float mouseSensitivity = 0.1f;

  bool firstMouse = true;

  std::vector<int> frame;

  friend class Raytracer;
};
