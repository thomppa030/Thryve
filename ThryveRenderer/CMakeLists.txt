cmake_minimum_required(VERSION 3.7 FATAL_ERROR)
project(ThryveRenderer LANGUAGES CXX)

# Set the C++ standard for the project
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use modern CMake practices for finding and linking to libraries
find_package(Vulkan REQUIRED)

add_compile_definitions(ENABLE_PROFILING)

# Define the library target
add_library(ThryveRenderer
        include/Core/Profiling.h
        src/Core/Profiling.cpp
        src/Layer.cpp
        include/Layer.h
        include/LayerStack.cpp
        include/LayerStack.h
        src/imGui/imGuiLayer.cpp
        include/imGui/imGuiLayer.h
        src/Vulkan/VulkanImGuiLayer.cpp
        include/Vulkan/VulkanImGuiLayer.h
        src/Core/Camera.cpp
        include/Core/Camera.h
        include/Vulkan/VulkanCommandQueueManager.cpp
        include/Vulkan/VulkanCommandQueueManager.h
        include/Vulkan/VulkanBuffer.cpp
        include/Vulkan/VulkanBuffer.h)

# Precompiled header
set(PRECOMPILE_HEADER "include/pch.h")
target_precompile_headers(ThryveRenderer PRIVATE ${PRECOMPILE_HEADER})

# Include directories for the ThryveRenderer target
target_include_directories(ThryveRenderer
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
        PRIVATE
        ${Vulkan_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/external/stb
        ${CMAKE_CURRENT_SOURCE_DIR}/external/tinyobjloader
        ${CMAKE_CURRENT_SOURCE_DIR}/external/VulkanMemoryAllocator/include
)

# Source and header files for the ThryveRenderer target
file(GLOB_RECURSE SOURCES "src/*cpp")
file(GLOB_RECURSE HEADERS
        "include/*h"
        "include/*hpp")
target_sources(ThryveRenderer PRIVATE ${SOURCES} ${HEADERS})

# Link libraries to the ThryveRenderer target
target_link_libraries(ThryveRenderer
        PRIVATE
        glfw
        spdlog::spdlog
        VulkanMemoryAllocator
        glm
)

# Add external dependencies as subdirectories
add_subdirectory(external/VulkanMemoryAllocator)
add_subdirectory(external/glm)

# Add necessary GLM definitions
target_compile_definitions(ThryveRenderer PRIVATE GLM_FORCE_INLINE GLM_ENABLE_EXPERIMENTAL GLM_FORCE_ALIGNED_GENTYPES)
