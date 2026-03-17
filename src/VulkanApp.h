#pragma once
#include <vulkan/vulkan.h>
#include "Window.h"

class VulkanApp {
    public:
        void init(Window& window);
        void cleanup();
    private:
        void createInstance();
        void createSurface(Window& window);
        void pickPhysicalDevice();
        void createLogicalDevice();

        VkInstance instance = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;

        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;

        VkQueue graphicsQueue = VK_NULL_HANDLE;
        uint32_t graphicsQueueFamilyIndex = 0;
};