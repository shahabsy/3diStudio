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
