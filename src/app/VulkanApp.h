#pragma once
#include "window/Window.h"
#include <renderer/VulkanContext.h>

class VulkanApp {
    public:
        void run();
    private:
        void init();
        void loop();
        void cleanup();
        
        Window window;
        VulkanContext context;
};