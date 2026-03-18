#include "VulkanContext.h"
#include <stdexcept>
#include <iostream>
#include <string>
#include <GLFW/glfw3.h>

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

    std::cout << "[INIT] vulkan initialized successfully\n";
}

void VulkanContext::cleanup() {
    // cleanup swapchain resources frist
    cleanupSwapChain();
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
}

void VulkanContext::render() {
    
}
