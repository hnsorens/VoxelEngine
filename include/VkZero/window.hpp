#pragma once

#include "VkZero/image.hpp"
#include <GLFW/glfw3.h>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace VkZero
{
  using WindowImpl = struct WindowImpl_T*;

  class Window {
  public:
    Window(int width, int height, const char *title);
    ~Window();
    
    bool isKeyPressed(int key);
    bool isMouseButtonPressed(int button);
    void hideCursor();
    double getDeltaTime() const;
    void getCursorPos(double *currentMouseX, double *currentMouseY);
    void showCursor();
    void getFramebufferSize(int *width, int *height);
    VkExtent2D getSwapChainExtent();
    AttachmentImage& getSwapChainImages();
    
    WindowImpl impl;
  };
}