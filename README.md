# 3diStudio - Vulkan Viewport Engine

A modern C++17 viewport engine built with Vulkan and GLFW, designed to grow into a Blender-level 3D viewport engine.

A viewport engine with real-time rendering system that provides:

1. **Core Visualization Pipeline**
   - 3D scene representation
   - Camera/view matrix management
   - Projection transformations
   - Geometry processing (vertex/fragment stages)

2. **Interactive Features**
   - Mouse/keyboard navigation (orbit/pan/zoom)
   - Viewport manipulators (gizmos/transform widgets)
   - Frame-rate independent movement

3. **Rendering Subsystems**
   ```mermaid
   graph LR
       A[Viewport Engine] --> B[Renderer Backend]
       A --> C[Scene Graph]
       A --> D[Input System]
       B --> E[Vulkan/D3D12/Metal]
       C --> F[Entity Components]

Technical Characteristics
60+ FPS performance
Low-latency input response
Adaptive quality (LOD, frustum culling)
GPU resource management
Comparison to Game Engines

Feature	               Viewport Engine	         Game Engine
Rendering Focus	       Primary	                 Secondary
Editor Integration     Required	                 Optional
Physics	               Minimal	                 Extensive
Asset Pipeline	       Direct Loading	         Cooked Data

1. Build System
CMake 3.16+ configuration
C++17 standard
Visual Studio 2022 (x64) support
Vulkan SDK 1.4.341.1 integration
GLFW and GLM libraries

2. Window Management
GLFW-based window creation (1280x720 default)
Window dimension tracking (getWidth(), getHeight())
Window resize detection with callback
Proper cleanup on exit

3. Vulkan Core
Instance creation with GLFW extensions
Surface creation (Win32/Vulkan bridge)
Physical device selection (GPU enumeration)
Logical device creation with graphics queue
Queue family detection (graphics + presentation support)
VK_KHR_SWAPCHAIN_EXTENSION enabled

4. Swapchain (Production-Grade)
SwapChainSupportDetails struct for hardware queries
querySwapChainSupport() - queries GPU capabilities
chooseSurfaceFormat() - prefers SRGB format
choosePresentMode() - prefers MAILBOX (triple buffering)
chooseExtent() - clamped to min/max capabilities
Triple buffering support (minImageCount + 1)

5. Rendering Resources
Image views for all swapchain images
Render pass with proper layout transitions
Framebuffers (one per swapchain image)
All handles initialized to VK_NULL_HANDLE

6. Window Resize Handling
framebufferResized flag
recreateSwapChain() method
Resize callback registered with GLFW

7. Render Loop
Event polling
Frame rendering call
Resize detection integration
Technology Stack
Component	   Technology
Language	      C++17
Build System	CMake 3.16+
Graphics API	Vulkan 1.2
Window Library	GLFW 3.4+ (built from source)
Math Library	GLM 1.0+
IDE	         Visual Studio 2022
Platform	      Windows 11 (x64)

Build Steps

powershell

Copy
# Clone repository (when applicable)
git clone https://github.com/your-repo/3diStudio.git
cd 3diStudio


# Configure (Debug build)
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64

# Compile
cmake --build .

# Alternative: Release build
cmake --build .