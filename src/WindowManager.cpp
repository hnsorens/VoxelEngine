#include "WindowManager.hpp"
#include "VulkanContext.hpp"
#include <GLFW/glfw3.h>
#include <algorithm>
#include <memory>
#include <vulkan/vulkan_core.h>

static void framebufferResizeCallback(GLFWwindow *window, int width,
                                      int height) {
  auto windowManager =
      reinterpret_cast<WindowManager *>(glfwGetWindowUserPointer(window));
  windowManager->framebufferResized = true;
}

static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
  if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
    glfwSetWindowShouldClose(window, GLFW_TRUE);
  }
}

WindowManager::WindowManager(int width, int height, const char *title) {
  glfwInit();

  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

  window = glfwCreateWindow(width, height, "Voxels", nullptr, nullptr);
  // const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
  // glfwSetWindowPos(window, (mode->width - WIDTH) / 2, 32);

  glfwSetWindowUserPointer(window, this);
  glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
  glfwSetKeyCallback(window, keyCallback);
}

WindowManager::~WindowManager() {
  glfwDestroyWindow(window);

  glfwTerminate();
}

void WindowManager::pollEvents() {
  deltaTime = glfwGetTime() - lastTime;
  lastTime = glfwGetTime();
  glfwPollEvents();
}

bool WindowManager::shouldClose() const {
  return glfwWindowShouldClose(window);
}

GLFWwindow *WindowManager::getWindow() const { return window; }

void WindowManager::recreateWindow() {
  int width = 0, height = 0;
  glfwGetFramebufferSize(window, &width, &height);
  while (width == 0 || height == 0) {
    glfwGetFramebufferSize(window, &width, &height);
    glfwWaitEvents();
  }
}

bool WindowManager::isKeyPressed(int key) {
  return glfwGetKey(window, key) == GLFW_PRESS;
}

bool WindowManager::isMouseButtonPressed(int button) {
  return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

void WindowManager::hideCursor() {
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

double WindowManager::getDeltaTime() const { return deltaTime; }

void WindowManager::getCursorPos(double *currentMouseX, double *currentMouseY) {
  glfwGetCursorPos(window, currentMouseX, currentMouseY);
}

void WindowManager::showCursor() {
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

void WindowManager::getFramebufferSize(int *width, int *height) {
  glfwGetFramebufferSize(window, width, height);
}