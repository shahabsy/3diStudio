#pragma once
#include <GLFW/glfw3.h>

class Window {
public:
    void create(int width, int height, const char *title);
    void destroy();

    bool shouldClose();
    void pollEvents();

    GLFWwindow* getNative() { return window; }

    int getWidth() { return width; }
    int getHeight() { return height; }
private:
    GLFWwindow *window = nullptr;
    int width = 0;
    int height = 0;
};