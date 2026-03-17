#include "Window.h"
#include "VulkanApp.h"
#include <iostream>

int main() {
    Window window;
    VulkanApp app;

    try {
        window.create(1200, 720, "Vulkan Window");
        app.init(window);

        while (!window.shouldClose()) {
            window.pollEvents();
        }
        app.cleanup();
        window.destroy();
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}