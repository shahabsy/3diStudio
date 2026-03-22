cmake_minimum_required(VERSION 3.16)
project(3diStudio)

set(CMAKE_CXX_STANDARD 17)

# Explicitly set Vulkan SDK path
set(VULKAN_SDK "C:/VulkanSDK/1.4.341.1")
set(Vulkan_INCLUDE_DIR "${VULKAN_SDK}/Include")
set(Vulkan_LIBRARY "${VULKAN_SDK}/Lib/vulkan-1.lib")

find_package(Vulkan REQUIRED)

add_subdirectory(glfw)

include_directories(
    src
    glfw/include
    glm
    ${Vulkan_INCLUDE_DIR}
)

file(GLOB_RECURSE SRC_FILES src/*.cpp)
add_executable(3diStudio ${SRC_FILES})

target_link_libraries(3diStudio
    glfw
    Vulkan::Vulkan
)

# Shader compilation
find_program(GLSLANG_VALIDATOR glslangvalidator REQUIRED)

# Compile vertex shader
add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/src/shaders/vert.spv
    COMMAND ${GLSLANG_VALIDATOR} -V ${CMAKE_SOURCE_DIR}/src/shaders/shader.vert -o ${CMAKE_SOURCE_DIR}/src/shaders/vert.spv
    DEPENDS ${CMAKE_SOURCE_DIR}/src/shaders/shader.vert
    COMMENT "Compiling vertex shader to SPIR-V"
    VERBATIM
)

# Compile fragment shader
add_custom_command(
    OUTPUT ${CMAKE_SOURCE_DIR}/src/shaders/frag.spv
    COMMAND ${GLSLANG_VALIDATOR} -V ${CMAKE_SOURCE_DIR}/src/shaders/shader.frag -o ${CMAKE_SOURCE_DIR}/src/shaders/frag.spv
    DEPENDS ${CMAKE_SOURCE_DIR}/src/shaders/shader.frag
    COMMENT "Compiling fragment shader to SPIR-V"
    VERBATIM
)

# Create custom targets for shaders
add_custom_target(vert_shader ALL
    DEPENDS ${CMAKE_SOURCE_DIR}/src/shaders/vert.spv
    COMMENT "Building vertex shader"
)

add_custom_target(frag_shader ALL
    DEPENDS ${CMAKE_SOURCE_DIR}/src/shaders/frag.spv
    COMMENT "Building fragment shader"
)

# Make 3diStudio depend on shader targets
add_dependencies(3diStudio vert_shader frag_shader)




