#include "app/VulkanApp.h"
#include <iostream>

void VulkanApp::run() {
    init();
    loop();
    cleanup();
}

void VulkanApp::init() {
    window.create(1280, 720, "3diStudio");

    // Set user pointer to window (for resize callback)
    glfwSetWindowUserPointer(window.getNative(), &window);
    context.init(&window);
}

void VulkanApp::loop() {
    std::cout << "[VK LOOP] Entering main loop..." << std::endl;
    int frameCount = 0;
    while (!window.shouldClose()) {
        try {
            window.pollEvents();
            
            // Check for window resize and recreate swapchain if needed
            if (window.wasResized()) {
                window.clearResized();
                context.recreateSwapChain();
            }
            
            context.render();
            frameCount++;
            if (frameCount % 60 == 0) {
                std::cout << "[VK LOOP] Frame " << frameCount << std::endl;
            }
        } catch (const std::exception &e) {
            std::cerr << "Frame Error: " << e.what() << std::endl;
            break;
        }
    }
    std::cout << "[VK LOOP] Exiting main loop after " << frameCount << " frames" << std::endl;
}

void VulkanApp::cleanup() {
    context.cleanup();
    window.destroy();
}