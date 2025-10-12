#pragma once

#include "VkZero/image.hpp"

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
    void pollEvents();
    bool shouldClose();
    uint32_t getWidth();
    uint32_t getHeight();
    AttachmentImage& getSwapChainImages();
    
    WindowImpl impl;
  };
}