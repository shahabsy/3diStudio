#pragma once
#include <GLFW/glfw3.h>

class Window {
public:
    void create(int width, int height, const char *title);
    void destroy();

    bool shouldClose();
    void pollEvents();

    GLFWwindow* getNative() { return window; }
private:
    GLFWwindow *window = nullptr;
};