#pragma once
#include <GLFW/glfw3.h>

class WindowManager {
public:
    WindowManager(int width, int height, const char* title);
    ~WindowManager();

    void pollEvents();
    bool shouldClose() const;
    void setFramebufferResizeCallback(void (*callback)(GLFWwindow*, int, int));
    void recreateWindow();
    GLFWwindow* getWindow() const;
    double getDeltaTime() const;
    bool isKeyPressed(int key);
    bool isMouseButtonPressed(int button);
    void hideCursor();
    void showCursor();

    bool framebufferResized = false;

private:
    GLFWwindow* window;

    double deltaTime = 0;
    double lastTime = 0;
}; 