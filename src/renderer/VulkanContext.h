#pragma once
#include <vulkan/vulkan.h>
#include <vector>
#include <algorithm>
#include <cstring>
#include "window/Window.h"
#include <glm/glm.hpp>

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities; // Basic surface capabilities
    std::vector<VkSurfaceFormatKHR> formats; // Available color formats and color spaces
    std::vector<VkPresentModeKHR> presentModes; // Available presentation modes
};

// Vertex data for triangle 
struct Vertex {
    glm::vec2 position;
    glm::vec3 color;
};

class VulkanContext {
    public:
        void init(Window* window);
        void cleanup();
        bool wasFrameBufferResized() { return framebufferResized; }
        void clearFrameBuffersResized() { framebufferResized = false; }
        void render();
        void createShaders();
        void createGraphicsPipeline();
        void recreateSwapChain();
        // set by the window resize callback to indicate spawchain recreation is needed
        bool framebufferResized = false;
    
    private:
        void createInstance();
        void createSurface(Window* window);
        void pickPhysicalDevice();
        void createLogicalDevice();

        uint32_t findQueueFamily();

        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

        VkSurfaceFormatKHR chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats);
        VkPresentModeKHR choosePresentMode(const std::vector<VkPresentModeKHR>& modes);
        VkExtent2D chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);



        void createSwapChain();
        void createImageViews();
        void createRenderPass();
        void createFramebuffers();

        void cleanupSwapChain();

        // Shader module creation helper
        VkShaderModule createShaderModule(const std::vector<char>& code);

        // Shader handles
        VkShaderModule vertShaderModule = VK_NULL_HANDLE;
        VkShaderModule fragShaderModule = VK_NULL_HANDLE;

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

        // Graphics pipeline
        VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
        VkPipeline graphicsPipeline = VK_NULL_HANDLE;

        // Command buffers
        VkCommandPool commandPool = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> commandBuffers;

        // vertex buffer
        VkBuffer vertexBuffer = VK_NULL_HANDLE;
        VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;
        std::vector<Vertex> vertices;

        void createCommandPool();
        void createCommandBuffers();

        void createVertexData();
        void createVertexBuffer();
        uint32_t findMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties);
        void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
};