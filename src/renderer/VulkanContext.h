#pragma once
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>
#include <vector>
#include "window/Window.h"


class VulkanContext {
    public:
        void init(Window* window);
        void cleanup();
    
    private:
        bool createInstance();
        void createSurface(Window* window);
        void pickPhysicalDevice();
        void createLogicalDevice();

        void createSwapChain();
        void createImageViews();
        void createRenderPass();
        void createFramebuffers();

        uint32_t findQueueFamily();

        VkInstance instance = VK_NULL_HANDLE;
        VkSurfaceKHR surface = VK_NULL_HANDLE;
        VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
        VkDevice device = VK_NULL_HANDLE;
        VkQueue graphicsQueue = VK_NULL_HANDLE;
        VkQueue presentQueue = VK_NULL_HANDLE;

        uint32_t windowWidth = 0;
        uint32_t windowHeight = 0;

        VkSwapchainKHR swapChain = VK_NULL_HANDLE;
        std::vector<VkImage> swapchainImages;
        VkFormat swapchainImageFormat;
        VkExtent2D swapchainExtent;

        std::vector<VkImageView> imageViews;
        
        VkRenderPass renderPass = VK_NULL_HANDLE;
        std::vector<VkFramebuffer> framebuffers;
};