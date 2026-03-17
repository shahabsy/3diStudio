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