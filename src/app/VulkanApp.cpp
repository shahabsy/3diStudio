#include "app/VulkanApp.h"
#include <iostream>

void VulkanApp::run() {
    init();
    loop();
    cleanup();
}

void VulkanApp::init() {
    window.create(1280, 720, "3diStudio");

    // Set user pointer and register callback
    glfwSetWindowUserPointer(window.getNative(), &context);
    context.init(&window);
}

void VulkanApp::loop() {
    while (!window.shouldClose()) {
        try {
            window.pollEvents();
            context.render();
            //std::cout << "Running frame..." << std::endl;
        } catch (const std::exception &e) {
            std::cerr << "Frame Error: " << e.what() << std::endl;
            break;
        }
    }
}

void VulkanApp::cleanup() {
    context.cleanup();
    window.destroy();
}