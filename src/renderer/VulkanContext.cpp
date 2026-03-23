#include "VulkanContext.h"
#include <stdexcept>
#include <iostream>
#include <string>
#include <GLFW/glfw3.h>
#include <fstream>

void VulkanContext::init(Window* window) {
    windowWidth = static_cast<uint32_t>(window->getWidth());
    windowHeight = static_cast<uint32_t>(window->getHeight());
    std::cout << "[INIT] Window size: " << windowWidth << " x " << windowHeight << std::endl;
    
    // contection to vulkan
    createInstance();
    // surface connect to window
    createSurface(window);
    // for physical device selection (GPU)
    pickPhysicalDevice();
    // logical device interface to GPU
    createLogicalDevice();

    // presentation images
    createSwapChain();
    // image views - wrappers around swapchain images
    createImageViews();
    // pipeline - shaders, vertex format etc. / render pass and rendering structure
    createRenderPass();
    // framebuffer - attach color and depth buffer to it - render targets
    createFramebuffers();

    // shaders
    createShaders();

    createGraphicsPipeline();

    // Command pool must be created BEFORE vertex buffer (copyBuffer needs it)
    createCommandPool();
    createCommandBuffers();

    createVertexData();
    createVertexBuffer();

    std::cout << "[INIT] vulkan initialized successfully\n";
}

void VulkanContext::cleanup() {
    // cleanup swapchain resources frist
    cleanupSwapChain();

    if (vertexBuffer != VK_NULL_HANDLE) {
        vkDestroyBuffer(device, vertexBuffer, nullptr);
    }
    if (vertexBufferMemory != VK_NULL_HANDLE) {
        vkFreeMemory(device, vertexBufferMemory, nullptr);
    }

    // destroy command buffers and pool
    if (commandPool != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device, commandPool, nullptr);
        std::cout << "[CLEANUP] command pool destroyed\n";
    }

    // destroy shaders
    if (vertShaderModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(device, vertShaderModule, nullptr);
        std::cout << "[CLEANUP] vertex shader module destroyed\n";
    }
    if (fragShaderModule != VK_NULL_HANDLE) {
        vkDestroyShaderModule(device, fragShaderModule, nullptr);
        std::cout << "[CLEANUP] fragment shader module destroyed\n";
    }
    // destroy graphics pipeline
    if (graphicsPipeline != VK_NULL_HANDLE) {
        vkDestroyPipeline(device, graphicsPipeline, nullptr);
        std::cout << "[CLEANUP] graphics pipeline destroyed\n";
    }
    if (pipelineLayout != VK_NULL_HANDLE) {
        vkDestroyPipelineLayout(device, pipelineLayout, nullptr);
        std::cout << "[CLEANUP] pipeline layout destroyed\n";
    }
    // destroy render pass
    if(renderPass != VK_NULL_HANDLE) {
        vkDestroyRenderPass(device, renderPass, nullptr);
        std::cout << "[CLEANUP] render pass destroyed\n";
    }

    // destroy logical device - this also destroys queues
    if (device != VK_NULL_HANDLE) {
        vkDestroyDevice(device, nullptr);
        std::cout << "[CLEANUP] logical device destroyed\n";
    }

    // destroy surface
    if (surface != VK_NULL_HANDLE) {
        vkDestroySurfaceKHR(instance, surface, nullptr);
        std::cout << "[CLEANUP] window surface destroyed\n";
    }

    // destroy instance
    if (instance != VK_NULL_HANDLE) {
        vkDestroyInstance(instance, nullptr);
        std::cout << "[CLEANUP] vulkan instance destroyed\n";
    }
}

void VulkanContext::createInstance() {
    // app info 
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "3diStudio";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Custom Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_2;
    // extensions required by glfw
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    std::cout << "[INSTANCE] required extenions (" << glfwExtensionCount << ") are \n";
    for (uint32_t i = 0; i < glfwExtensionCount; i++) {
        std::cout << "\t" << glfwExtensions[i] << "\n";
    }

    // create instance info
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    // enable GLFW extensions
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    // validation layers could be enabled here
    createInfo.enabledLayerCount = 0;

    // create the vulkan instance
    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create vulkan instance" + std::to_string(result));
    }
    std::cout << "[INSTANCE] vulkan instance created successfully\n";
}

void VulkanContext::createSurface(Window* window) {
    // GLFW to create a surface bound to the window
    if (glfwCreateWindowSurface(instance, window->getNative(), nullptr, &surface) != VK_SUCCESS) {
        throw std::runtime_error("failed to create window surface");
    }
    std::cout << "[SURFACE] window surface created successfully\n";
}

void VulkanContext::pickPhysicalDevice() {
    // get number of available physical GPUs
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

    if (deviceCount == 0) {
        throw std::runtime_error("failed to find any GPU");
    }
    std::cout << "[PHYSICAL DEVICE] found " << deviceCount << " GPU(s)\n";

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

    // select the first GPU available
    physicalDevice = devices[0];

    VkPhysicalDeviceProperties properties;
    vkGetPhysicalDeviceProperties(physicalDevice, &properties);
    std::cout << "[PHYSICAL DEVICE] " << properties.deviceName << "\n";
}

void VulkanContext::createLogicalDevice() {
    // find a queue family that supports graphics
    uint32_t queueFamilyIndex = findQueueFamily();

    // create queue with priority 1.0(highest)
    float priority = 1.0f;

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = 0;
    queueCreateInfo.queueCount = 1;
    queueCreateInfo.pQueuePriorities = &priority;

    // enable required swapchain extensions
    const char* deviceExtensions[] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

    // device creation information
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = 1;  // &queueCreateInfo;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    // enable required device extensions
    createInfo.enabledExtensionCount = 1;  // &deviceExtensions[0];
    createInfo.ppEnabledExtensionNames = deviceExtensions;

    // validation layers could be enabled here - currently disabled
    createInfo.enabledLayerCount = 0;

    // create teh logical device
    VkResult res = vkCreateDevice(physicalDevice, &createInfo, nullptr, &device);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("failed to create logical device");
    }
    // get the graphics queue
    vkGetDeviceQueue(device, 0, 0, &graphicsQueue);
    std::cout << "[LOGICAL DEVICE] created successfully with queue family: " << queueFamilyIndex << "\n";
}

uint32_t VulkanContext::findQueueFamily() {
    // get number of queue families
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

    // get properties of all queue families
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

    std::cout << "[QUEUE] found " << queueFamilyCount << " queue families\n";
    // iterate over all queue families and find one that supports graphics operations
    // and presentation to our surface
    for (uint32_t i = 0; i < queueFamilies.size(); ++i) {
        // Check if this queue family supports graphics operations
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            // check if this queue family supports presentation to our surface
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupport);
            
            if (presentSupport) {
                std::cout << "[QUEUE] queue family " << i << " supports presentation\n";
                return i;
            }
        }
    }
    // didn't find a queue family that supports both graphics and presentation
    throw std::runtime_error("failed to find a suitable queue family with graphics and presentation support");
}

SwapChainSupportDetails VulkanContext::querySwapChainSupport(VkPhysicalDevice device) {
    SwapChainSupportDetails details;
    // Query basic surface capabilities
    //vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);

    // Query available surface formats
    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    // Query available present modes
    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }
    std::cout << "[SWAPCHAIN] found " << formatCount << " formats," << presentModeCount << " present modes\n";

    return details;
}

VkSurfaceFormatKHR VulkanContext::chooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& formats) {
    // Prefer SRGB format for proper color presentation
    for (const auto& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
            format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            std::cout << "[SWAPCHAIN] found SRGB format\n";
            return format;
        }
    }
    // If preferred format not available, return the first one
    std::cout << "[SWAPCHAIN] using first format\n";
    return formats[0];
}

VkPresentModeKHR VulkanContext::choosePresentMode(const std::vector<VkPresentModeKHR>& modes) {
    for (const auto& mode : modes) {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR) {
            std::cout << "[SWAPCHAIN] found MAILBOX present mode\n";
            return mode;
        }
    }
    std::cout << "[SWAPCHAIN] using FIFO present mode\n";
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanContext::chooseExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    // Check if the extent is already set by the surface
    if (capabilities.currentExtent.width != UINT32_MAX) {
        std::cout << "[SWAPCHAIN] using current extent:"
        << capabilities.currentExtent.width << " x " << capabilities.currentExtent.height << "\n";
        return capabilities.currentExtent;
    }
    // clamp extent to min/max values from capabilities
    // this presents validation errors and crashes when the window is resized
    VkExtent2D extent = {this->windowWidth, this->windowHeight};

    extent.width = std::clamp(extent.width,
                              capabilities.minImageExtent.width,
                              capabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height,
                               capabilities.minImageExtent.height,
                               capabilities.maxImageExtent.height);
    
    std::cout << "[SWAPCHAIN] using clamped extent:" << extent.width << " x " << extent.height << "\n";
    return extent;
}

VkShaderModule VulkanContext::createShaderModule(const std::vector<char>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;//_KHR;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    VkShaderModule shaderModule;
    if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
        throw std::runtime_error("failed to create shader module");
    }

    return shaderModule;
}

void VulkanContext::createShaders() {
    // Load vertex shader
    std::ifstream vertFile("../../src/shaders/vert.spv", std::ios::ate | std::ios::binary);
    if (!vertFile.is_open()) {
        throw std::runtime_error("failed to open vert.spv");
    }

    size_t vertSize = vertFile.tellg();
    std::vector<char> vertCode(vertSize);
    vertFile.seekg(0);
    vertFile.read(vertCode.data(), vertSize);
    vertFile.close();

    // Load fragment shader
    std::ifstream fragFile("../../src/shaders/frag.spv", std::ios::ate | std::ios::binary);
    if (!fragFile.is_open()) {
        throw std::runtime_error("failed to open frag.spv");
    }

    size_t fragSize = fragFile.tellg();
    std::vector<char> fragCode(fragSize);
    fragFile.seekg(0);
    fragFile.read(fragCode.data(), fragSize);
    fragFile.close();

    // Create shader modules from bytecode
    vertShaderModule = createShaderModule(vertCode);
    fragShaderModule = createShaderModule(fragCode);

    std::cout << "[SHADERS] loaded vertex and fragment shaders\n";
}

void VulkanContext::createGraphicsPipeline() {
    // Vertex shader stage
    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    // Fragment shader stage
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    // Shader stages array
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // Vertex input - vertex data format
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Vertex);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    VkVertexInputAttributeDescription attributeDescription[2];
    // position attribute
    attributeDescription[0].binding = 0;
    attributeDescription[0].location = 0;
    attributeDescription[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescription[0].offset = offsetof(Vertex, position);

    // color attribute
    attributeDescription[1].binding = 0;
    attributeDescription[1].location = 1;
    attributeDescription[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescription[1].offset = offsetof(Vertex, color);

    // Vertex Input
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = 2;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescription;

    // Input Assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Viewport And Scissor
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float)swapchainExtent.width;
    viewport.height = (float)swapchainExtent.height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = swapchainExtent;

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;

    // Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    //rasterizer.flatShadingEnable = VK_FALSE;

    // NultiSampling
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

    // Color Blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                                          VK_COLOR_COMPONENT_G_BIT |
                                          VK_COLOR_COMPONENT_B_BIT |
                                          VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // Pipeline layout
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0; // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr; // Optional
    
    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
        throw std::runtime_error("failed to create pipeline layout");
    }

    // Create Pipeline
    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pColorBlendState = &colorBlending;
    //pipelineInfo.pDynamicState = nullptr;
    pipelineInfo.layout = pipelineLayout;
    pipelineInfo.renderPass = renderPass;
    pipelineInfo.subpass = 0;
    //pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
    //pipelineInfo.basePipelineIndex = -1;

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS) {
        throw std::runtime_error("failed to create graphics pipeline");
    }

    std::cout << "[GRAPHICS PIPELINE] created graphics pipeline\n";
}

void VulkanContext::createCommandBuffers() {
    // Allocate command buffers
    commandBuffers.resize(swapchainImages.size());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t)commandBuffers.size();

    if (vkAllocateCommandBuffers(device, &allocInfo, &commandBuffers[0]) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers");
    }

    std::cout << "[COMMAND BUFFER] allocated command buffers\n";
}

void VulkanContext::createCommandPool() {
    // Find queue family with graphics support
    uint32_t queueFamilyIndex = findQueueFamily();

    // Create command pool
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    poolInfo.queueFamilyIndex = queueFamilyIndex;

    if (vkCreateCommandPool(device, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
        throw std::runtime_error("failed to create command pool");
    }

    std::cout << "[COMMAND POOL] created command pool\n";
}

void VulkanContext::createSwapChain() {
    // Query hardware capabilities and supported formats/modes
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    // check if swapchain is supported
    if (swapChainSupport.formats.empty() || swapChainSupport.presentModes.empty()) {
        throw std::runtime_error("no suitable swap chain format or present mode found");
    }

    // Choose optional settings based on hardware capabilities
    VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = choosePresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseExtent(swapChainSupport.capabilities);

    // Determine number of images in swapchain
    // use minImage + 1 for triple buffering support
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    // Don't exceed max limit
    if (swapChainSupport.capabilities.maxImageCount > 0 &&
        imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    std::cout << "[SWAPCHAIN] creating swap chain with " << imageCount << " images\n";

    // Create swap chain
    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Queuue family sharing (simplified - using single queue family)
    uint32_t queueFamilyIndex = findQueueFamily();
    createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    createInfo.queueFamilyIndexCount = 1;
    createInfo.pQueueFamilyIndices = &queueFamilyIndex;
    
    // Pre-transform
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;

    // presentation mode (VSync or triple buffering)
    createInfo.presentMode = presentMode;

    // clip pixels that are abscured by other windows
    createInfo.clipped = VK_TRUE;

    // old swapchain is no longer needed
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    // create teh swapchian
    VkResult res = vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapChain);
    if (res != VK_SUCCESS) {
        std::cout << "[ERROR] vkCreateSwapchainKHR failed with code: " << res << std::endl;
        if (res == VK_ERROR_EXTENSION_NOT_PRESENT) {
            std::cout << "[ERROR] VK_KHR_swapchain extension not enabled!" << std::endl;
        }
        throw std::runtime_error("failed to create swap chain!");
    }
    
    std::cout << "[VULKAN] Swapchain created successfully" << std::endl;

    // Retrieve swapchian images
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(device, swapChain, &imageCount, swapchainImages.data());

    // save swapchain properties
    swapchainImageFormat = surfaceFormat.format;
    swapchainExtent = extent;

    std::cout << "[SWAPCHAIN] Retrieved " << imageCount << " images\n";
}

void VulkanContext::createImageViews() {
    // Create one image view per swapchain image
    imageViews.resize(swapchainImages.size());

    for (size_t i = 0; i < swapchainImages.size(); ++i) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapchainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapchainImageFormat;
        
        // color mapping
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        // create image view
        VkResult result = vkCreateImageView(device, &createInfo, nullptr, &imageViews[i]);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create image view!");
        }
    }
    std::cout << "[VULKAN] Created " << imageViews.size() << " image views\n";
}

void VulkanContext::createRenderPass() {
    // Single color attachment description
    VkAttachmentDescription color{};
    color.format = swapchainImageFormat;
    color.samples = VK_SAMPLE_COUNT_1_BIT;

    // what to do with attachment at start of render pass
    color.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;  // clear to black
    // what to do with attachment at the end of render pass
    color.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // keep for presentation
    // Stencil operations (not used)
    color.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    color.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    // Must set initial layout to avoid validation errors
    // This tells Vulkan that the image doesn't have a meaningful layout yet
    color.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    // Final layout after render pass ( ready for presentation)
    color.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    // Attachment reference for the subpass
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapchainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    // Single subpass description
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    // Subpass dependency
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    // Render pass creation information
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &color;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    // Create the reneder pass
    VkResult result = vkCreateRenderPass(device, &renderPassInfo, nullptr, &renderPass);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void VulkanContext::createFramebuffers() {
    // Crate one framebuffer per image view
    framebuffers.resize(imageViews.size());

    for (size_t i = 0; i < imageViews.size(); ++i) {
        // Attachments for this framebuffer (color attachment)
        VkImageView attachments[] = {imageViews[i]};

        // Framebuffer creation information
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchainExtent.width;
        framebufferInfo.height = swapchainExtent.height;

        // Must set layers = 1
        //this specifies the number of layers in the framebuffer images
        // without this, validation layers will complain
        framebufferInfo.layers = 1;

        // Create the framebuffer
        VkResult result = vkCreateFramebuffer(device, &framebufferInfo, nullptr, &framebuffers[i]);
        if (result != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
    std::cout << "[VULKAN] Created " << framebuffers.size() << " framebuffers\n";
}

void VulkanContext::cleanupSwapChain() {
    // Destroy all framebuffer
    for (auto framebuffer : framebuffers) {
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    }
    framebuffers.clear();

    // Destroy all image views
    for (auto imageView : imageViews) {
        vkDestroyImageView(device, imageView, nullptr);
    }
    imageViews.clear();

    // Destroy swapchain (images are destroyed automatically)
    if (swapChain != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device, swapChain, nullptr);
        swapChain = VK_NULL_HANDLE;
    }
    std::cout << "[VULKAN] Cleaned up swap chain\n";

    // Destroy the render pass
    //vkDestroyRenderPass(device, renderPass, nullptr);
}

void VulkanContext::recreateSwapChain() {
    std::cout << "[VULKAN] Recreating swap chain..." << std::endl;
    // Clean up the old swap chain
    cleanupSwapChain();

    //create new swapChain with updated dimensions
    createSwapChain();
    createImageViews();
    //createRenderPass();
    createFramebuffers();

    // Recreate pipeline with new dimensions
    createGraphicsPipeline();
}

void VulkanContext::render() {
    // Check for window resize at start of frame
    if (framebufferResized) {
        framebufferResized = false;
        recreateSwapChain();
        return;
    }

    // 1. Acquire image from swap chain
    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(
        device,
        swapChain,
        UINT64_MAX,
        VK_NULL_HANDLE,
        VK_NULL_HANDLE,
        &imageIndex
    );
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    // 2. Reset Command buffer
    vkResetCommandBuffer(commandBuffers[imageIndex], 0);

    // 3. Begin recording command buffer
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    // 4. Begin render pass
    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = renderPass;
    renderPassInfo.framebuffer = framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = swapchainExtent;

    // Clear color attachment to black
    VkClearValue clearColor = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    // 5. Draw a triangle
    vkCmdBindPipeline(commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

    // Bind ver buffer
    VkBuffer vertexBuffers[] = {vertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffers[imageIndex], 0, 1, vertexBuffers, offsets);

    // 6. Drawn 3 vertices or triangle
    vkCmdDraw(commandBuffers[imageIndex], 3, 1, 0, 0);

    // 7. End render pass and command buffer
    vkCmdEndRenderPass(commandBuffers[imageIndex]);

    // 8 End command buffer recording
    if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

    // 9. Submit command buffer
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffers[imageIndex];

    if (vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    //10. Present rendered image to swap chain
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    //presentInfo.waitSemaphoreCount = 1;
    //presentInfo.pWaitSemaphores = &imageAvailableSemaphore;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapChain;
    presentInfo.pImageIndices = &imageIndex;

    vkQueuePresentKHR(graphicsQueue, &presentInfo);
    /*
    result = vkQueuePresentKHR(presentQueue, &presentInfo);

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        recreateSwapChain();
    } else if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to present swap chain image!");
    }

    // Wait for the device to finish rendering before continuing
    vkDeviceWaitIdle(device);
    */
}

void VulkanContext::createVertexData() {
    vertices = {
        // Position (x,y)    Color(r,g,b)
        {{-0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},
        {{0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}},
        {{0.0f,  0.5f}, {1.0f, 1.0f, 1.0f}}
    };
    std::cout << "triangle vertex data created: " << vertices.size() << std::endl;
}

uint32_t VulkanContext::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) &&
            (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }

    throw std::runtime_error("Failed to find suitable memory type!");
}

void VulkanContext::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    // Create a command buffer for the copy
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = commandPool;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    vkBeginCommandBuffer(commandBuffer, &beginInfo);

    VkBufferCopy copyRegion{};
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;

    vkQueueSubmit(graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
    vkQueueWaitIdle(graphicsQueue);

    vkFreeCommandBuffers(device, commandPool, 1, &commandBuffer);
}

void VulkanContext::createVertexBuffer() {
    std::cout << "[VERTEX] Creating vertex buffer..." << std::endl;
    VkDeviceSize bufferSize = sizeof(vertices[0]) * vertices.size();

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;
    
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    bufferInfo.size = bufferSize;
    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    if (vkCreateBuffer(device, &bufferInfo, nullptr, &stagingBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create staging buffer!");
    }
    std::cout << "[VERTEX] Staging buffer created" << std::endl;

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device, stagingBuffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, 
                                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | 
                                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &stagingBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate staging buffer memory!");
    }

    vkBindBufferMemory(device, stagingBuffer, stagingBufferMemory, 0);
    std::cout << "[VERTEX] Staging buffer memory bound" << std::endl;

    void* data;
    vkMapMemory(device, stagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), bufferSize);
    vkUnmapMemory(device, stagingBufferMemory);
    std::cout << "[VERTEX] Data copied to staging buffer" << std::endl;

    bufferInfo.usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    
    if (vkCreateBuffer(device, &bufferInfo, nullptr, &vertexBuffer) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create vertex buffer!");
    }
    std::cout << "[VERTEX] Vertex buffer created" << std::endl;

    vkGetBufferMemoryRequirements(device, vertexBuffer, &memRequirements);

    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, 
                                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    if (vkAllocateMemory(device, &allocInfo, nullptr, &vertexBufferMemory) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate vertex buffer memory!");
    }
    std::cout << "[VERTEX] Vertex buffer memory allocated" << std::endl;

    vkBindBufferMemory(device, vertexBuffer, vertexBufferMemory, 0);

    copyBuffer(stagingBuffer, vertexBuffer, bufferSize);
    std::cout << "[VERTEX] Data copied to vertex buffer" << std::endl;

    vkDestroyBuffer(device, stagingBuffer, nullptr);
    vkFreeMemory(device, stagingBufferMemory, nullptr);

    std::cout << "[VERTEX] Created vertex buffer\n";
}

