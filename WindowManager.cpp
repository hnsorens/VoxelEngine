#include "WindowManager.hpp"
#include <GLFW/glfw3.h>


static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    auto windowManager = reinterpret_cast<WindowManager*>(glfwGetWindowUserPointer(window));
    windowManager->framebufferResized = true;
}

WindowManager::WindowManager(int width, int height, const char* title) 
{
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    window = glfwCreateWindow(width, height, "Voxels", nullptr, nullptr);
    // const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    // glfwSetWindowPos(window, (mode->width - WIDTH) / 2, 32);

    glfwSetWindowUserPointer(window, this);
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

WindowManager::~WindowManager()
{
    glfwDestroyWindow(window);

    glfwTerminate();
}

void WindowManager::pollEvents() 
{
    deltaTime = glfwGetTime() - lastTime;
    lastTime = glfwGetTime();
    glfwPollEvents();
}

bool WindowManager::shouldClose() const 
{ 
    return glfwWindowShouldClose(window); 
}

void WindowManager::setFramebufferResizeCallback(void (*callback)(GLFWwindow*, int, int)) {}

GLFWwindow* WindowManager::getWindow() const 
{ 
    return window; 
} 

void WindowManager::recreateWindow()
{
    int width = 0, height = 0;
    glfwGetFramebufferSize(window, &width, &height);
    while (width == 0 || height == 0) {
        glfwGetFramebufferSize(window, &width, &height);
        glfwWaitEvents();
    }
}

double WindowManager::getDeltaTime() const
{
    return deltaTime;
}