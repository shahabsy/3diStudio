#include "Window.h"
#include <stdexcept>
#include <iostream>

// Forward declaration of glfwCreateWindow
//class VulkanContext;

static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
    void* userPtr = glfwGetWindowUserPointer(window);
    if (userPtr) {
        auto* win = static_cast<Window*>(userPtr);
        win->setFrameBufferResized(true);
        win->setWidth(width);
        win->setHeight(height);
        std::cout << "[GLFW] Framebuffer resized!" << std::endl;
    }
}

void Window::create(int width, int height, const char* title) {
    this->width = width;
    this->height = height;
    
    if (!glfwInit())
        throw std::runtime_error("Failed to initialize GLFW");
    
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    
    window = glfwCreateWindow(width, height, title, nullptr, nullptr);
    
    if (!window) {
        throw std::runtime_error("Failed to create window");
    }
    // register the resizze callback
    glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
}

void Window::destroy() {
    glfwDestroyWindow(window);
    glfwTerminate();
}

bool Window::shouldClose() {
    return glfwWindowShouldClose(window);
}

void Window::pollEvents() {
    glfwPollEvents();
}