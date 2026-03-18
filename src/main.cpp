#include "app/VulkanApp.h"
#include <iostream>

int main() {
    try {
        std::cout << "[APP] Starting 3diStudio..." << std::endl;
        VulkanApp app;
        app.run();
        std::cout << "[APP] Exited normally" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "[ERROR] " << e.what() << std::endl;
        std::cout << "Press Enter to close..." << std::endl;
        std::cin.get();
        return EXIT_FAILURE;
    }
    
    std::cout << "Press Enter to close..." << std::endl;
    std::cin.get();
    return EXIT_SUCCESS;
}