#include "Window.h"
#include <stdexcept>

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