#pragma once

#include <GLFW/glfw3.h>

class WindowManager {
public:
  WindowManager(int width, int height, const char *title);
  ~WindowManager();

  void pollEvents();
  bool shouldClose() const;
  void recreateWindow();
  GLFWwindow *getWindow() const;
  bool isKeyPressed(int key);
  bool isMouseButtonPressed(int button);
  void hideCursor();
  double getDeltaTime() const;
  void getCursorPos(double *currentMouseX, double *currentMouseY);
  void showCursor();
  void getFramebufferSize(int *width, int *height);

  bool framebufferResized = false;

private:
  GLFWwindow *window;
  double deltaTime = 0;
  double lastTime = 0;
};