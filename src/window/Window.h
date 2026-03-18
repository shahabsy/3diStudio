#pragma once
#include <GLFW/glfw3.h>

//static void framebufferResizeCallback(GLFWwindow* window, int width, int height);

class Window {
public:
    void create(int width, int height, const char *title);
    void destroy();

    bool shouldClose();
    void pollEvents();

    GLFWwindow* getNative() { return window; }

    int getWidth() { return width; }
    int getHeight() { return height; }



    // Setter for callback to use
    void setFrameBufferResized(bool value) { framebufferResized = value; }
    void setWidth(int w) {width = w;}
    void setHeight(int h) {height = h;}

    bool wasResized() { return framebufferResized; }
    void clearResized() { framebufferResized = false; }
private:
    GLFWwindow *window = nullptr;
    int width = 0;
    int height = 0;
    bool framebufferResized = false;
};